/**
 * @file server.cpp
 * @brief HTTP server implementation
 */

#include "crest/crest.h"
#include "crest/crest.hpp"
#include "crest/internal/app_internal.h"
#include "../utils/thread_pool.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <atomic>
#include <mutex>

extern "C" {
    void crest_log_info(const char* msg);
    void crest_log_success(const char* msg);
    void crest_log_error(const char* msg);
    void crest_log_request(const char* method, const char* path, int status);
}

#if defined(_WIN32) || defined(_WIN64) || defined(CREST_WINDOWS)
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
    #define strdup _strdup
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

static std::atomic<bool> server_running{false};

static const char* get_swagger_html(crest_app_t* app);
static const char* get_openapi_json(crest_app_t* app);
static void handle_client(SOCKET client_socket, crest_app_t* app);
static void parse_request(const char* buffer, crest_request_t* req);

extern "C" {

int crest_run(crest_app_t* app, const char* host, int port) {
    if (!app || !host) return -1;
    
#if defined(_WIN32) || defined(_WIN64) || defined(CREST_WINDOWS)
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return -1;
    }
#endif
    
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed\n");
        return -1;
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(host);
    address.sin_port = htons(port);
    
    if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed\n");
        closesocket(server_socket);
        return -1;
    }
    
    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed\n");
        closesocket(server_socket);
        return -1;
    }
    
    app->server_socket = server_socket;
    app->running = true;
    server_running = true;
    
    // Initialize thread pool with hardware concurrency
    size_t num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 8;
    app->thread_pool = new crest::ThreadPool(num_threads * 2);
    
    char msg[256];
    snprintf(msg, sizeof(msg), "Crest server running on http://%s:%d", host, port);
    crest_log_success(msg);
    snprintf(msg, sizeof(msg), "Thread pool initialized with %zu workers", num_threads * 2);
    crest_log_info(msg);
    
    if (app->docs_enabled) {
        snprintf(msg, sizeof(msg), "Documentation: http://%s:%d/docs", host, port);
        crest_log_info(msg);
        snprintf(msg, sizeof(msg), "Playground: http://%s:%d/playground", host, port);
        crest_log_info(msg);
    }
    
    while (server_running && app->running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket != INVALID_SOCKET) {
            auto* pool = static_cast<crest::ThreadPool*>(app->thread_pool);
            pool->enqueue([client_socket, app]() {
                handle_client(client_socket, app);
            });
        }
    }
    
    delete static_cast<crest::ThreadPool*>(app->thread_pool);
    app->thread_pool = nullptr;
    
    closesocket(server_socket);
#if defined(_WIN32) || defined(_WIN64) || defined(CREST_WINDOWS)
    WSACleanup();
#endif
    
    return 0;
}

void crest_stop(crest_app_t* app) {
    if (app) {
        app->running = false;
        server_running = false;
    }
}

} // extern "C"

static void handle_client(SOCKET client_socket, crest_app_t* app) {
    char buffer[8192] = {0};
    int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_read <= 0) {
        closesocket(client_socket);
        return;
    }
    
    crest_request_t req = {0};
    parse_request(buffer, &req);
    
    crest_response_t res = {0};
    res.status = 200;
    res.sent = false;
    
    // Handle docs routes only if docs are enabled
    bool is_docs_route = (strcmp(req.path, "/docs") == 0 || 
                          strcmp(req.path, "/openapi.json") == 0 || 
                          strcmp(req.path, "/playground") == 0);
    
    if (app->docs_enabled && is_docs_route) {
        if (strcmp(req.path, "/docs") == 0) {
            const char* html = get_swagger_html(app);
            crest_response_html(&res, 200, html);
        }
        else if (strcmp(req.path, "/openapi.json") == 0) {
            const char* json = get_openapi_json(app);
            crest_response_json(&res, 200, json);
        }
        else if (strcmp(req.path, "/playground") == 0) {
        const char* playground_html = 
            "<!DOCTYPE html><html><head><meta charset='utf-8'><title>API Playground</title><meta name='viewport' content='width=device-width,initial-scale=1'><style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:#fafafa;color:#333}.header{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:white;padding:40px 20px;position:relative;box-shadow:0 4px 6px rgba(0,0,0,0.1)}.header h1{font-size:2.5em;margin-bottom:10px;font-weight:600}.refresh-btn{position:absolute;top:20px;right:20px;background:rgba(255,255,255,0.2);border:2px solid white;color:white;padding:10px 20px;border-radius:6px;cursor:pointer;font-size:14px;font-weight:600;transition:all 0.3s}.refresh-btn:hover{background:rgba(255,255,255,0.3);transform:scale(1.05)}.container{max-width:1400px;margin:0 auto;padding:20px}.playground{background:white;padding:25px;margin:20px 0;border-radius:8px;box-shadow:0 2px 8px rgba(0,0,0,0.08)}.playground h2{color:#667eea;margin-bottom:20px}.form-group{margin:15px 0}.form-group label{display:block;margin-bottom:8px;font-weight:600;color:#333}.form-control{width:100%;padding:12px;border:1px solid #e0e0e0;border-radius:6px;font-size:1em;font-family:'Courier New',monospace}textarea.form-control{min-height:150px;resize:vertical}.btn-group{display:flex;gap:10px;margin:20px 0}.btn{padding:12px 24px;border:none;border-radius:6px;cursor:pointer;font-size:1em;font-weight:600;transition:all 0.3s}.btn-primary{background:#667eea;color:white}.btn-primary:hover{background:#5568d3;transform:translateY(-2px);box-shadow:0 4px 8px rgba(102,126,234,0.3)}.btn-secondary{background:#6c757d;color:white}.btn-secondary:hover{background:#5a6268}.response-box{margin-top:20px;padding:20px;background:#f8f9fa;border-radius:6px;border-left:4px solid #667eea;display:none}.response-box.show{display:block}.response-box.success{border-left-color:#49cc90}.response-box.error{border-left-color:#f93e3e}.response-header{display:flex;justify-content:space-between;margin-bottom:15px;padding-bottom:10px;border-bottom:2px solid #e0e0e0}.response-body{font-family:'Courier New',monospace;white-space:pre-wrap;word-wrap:break-word;background:white;padding:15px;border-radius:4px;max-height:400px;overflow-y:auto}.tabs{display:flex;gap:10px;margin-bottom:20px;border-bottom:2px solid #e0e0e0}.tab{padding:12px 24px;cursor:pointer;border-bottom:3px solid transparent;transition:all 0.3s;font-weight:600}.tab.active{border-bottom-color:#667eea;color:#667eea}.tab:hover{background:#f8f9fa}.tab-content{display:none}.tab-content.active{display:block}.header-item{display:flex;gap:10px;margin-bottom:10px}.header-item input{flex:1}.add-header-btn{background:#28a745;color:white;padding:8px 16px;border:none;border-radius:4px;cursor:pointer;font-size:0.9em}.add-header-btn:hover{background:#218838}.remove-btn{background:#dc3545;color:white;padding:8px 12px;border:none;border-radius:4px;cursor:pointer}.remove-btn:hover{background:#c82333}@media(max-width:768px){.header h1{font-size:1.8em}.container{padding:10px}.btn-group{flex-direction:column}}</style></head><body><div class='header'><button class='refresh-btn' onclick='location.reload()'>🔄 Refresh</button><h1>🎮 API Playground</h1><p>Test your API endpoints interactively</p></div><div class='container'><div class='playground'><h2>🚀 Request Builder</h2><div class='tabs'><div class='tab active' onclick='switchTab(\"basic\")'>Basic</div><div class='tab' onclick='switchTab(\"headers\")'>Headers</div><div class='tab' onclick='switchTab(\"body\")'>Body</div></div><div id='basic-tab' class='tab-content active'><div class='form-group'><label>HTTP Method</label><select id='method' class='form-control'><option value='GET'>GET</option><option value='POST'>POST</option><option value='PUT'>PUT</option><option value='DELETE'>DELETE</option><option value='PATCH'>PATCH</option></select></div><div class='form-group'><label>Endpoint URL</label><input type='text' id='url' class='form-control' placeholder='/api/endpoint' value='/'></div><div class='form-group'><label>Query Parameters (key=value, one per line)</label><textarea id='query' class='form-control' placeholder='page=1&#10;limit=10'></textarea></div></div><div id='headers-tab' class='tab-content'><div class='form-group'><label>Custom Headers</label><div id='headers-list'><div class='header-item'><input type='text' placeholder='Header Name' class='form-control'><input type='text' placeholder='Header Value' class='form-control'><button class='remove-btn' onclick='removeHeader(this)'>✕</button></div></div><button class='add-header-btn' onclick='addHeader()'>+ Add Header</button></div></div><div id='body-tab' class='tab-content'><div class='form-group'><label>Request Body (JSON)</label><textarea id='body' class='form-control' placeholder='{\"key\": \"value\"}'></textarea></div><button class='btn btn-secondary' onclick='formatJSON()'>Format JSON</button></div><div class='btn-group'><button class='btn btn-primary' onclick='sendRequest()'>▶ Send Request</button><button class='btn btn-secondary' onclick='clearForm()'>🗑 Clear</button></div></div><div id='response' class='response-box'><div class='response-header'><div><strong>Response</strong></div><div id='response-status'></div></div><div class='response-body' id='response-body'></div></div></div><script>function switchTab(tab){document.querySelectorAll('.tab').forEach(t=>t.classList.remove('active'));document.querySelectorAll('.tab-content').forEach(t=>t.classList.remove('active'));event.target.classList.add('active');document.getElementById(tab+'-tab').classList.add('active');}function addHeader(){const list=document.getElementById('headers-list');const item=document.createElement('div');item.className='header-item';item.innerHTML='<input type=\"text\" placeholder=\"Header Name\" class=\"form-control\"><input type=\"text\" placeholder=\"Header Value\" class=\"form-control\"><button class=\"remove-btn\" onclick=\"removeHeader(this)\">✕</button>';list.appendChild(item);}function removeHeader(btn){btn.parentElement.remove();}function formatJSON(){try{const body=document.getElementById('body');const json=JSON.parse(body.value);body.value=JSON.stringify(json,null,2);}catch(e){alert('Invalid JSON');}}function clearForm(){document.getElementById('url').value='/';document.getElementById('query').value='';document.getElementById('body').value='';document.getElementById('response').classList.remove('show','success','error');}async function sendRequest(){const method=document.getElementById('method').value;let url=document.getElementById('url').value;const query=document.getElementById('query').value;const body=document.getElementById('body').value;const responseBox=document.getElementById('response');const responseBody=document.getElementById('response-body');const responseStatus=document.getElementById('response-status');if(query){const params=query.split('\\n').filter(l=>l.trim()).map(l=>l.trim()).join('&');url+=url.includes('?')?'&'+params:'?'+params;}const headers={'Content-Type':'application/json'};document.querySelectorAll('#headers-list .header-item').forEach(item=>{const inputs=item.querySelectorAll('input');if(inputs[0].value&&inputs[1].value){headers[inputs[0].value]=inputs[1].value;}});responseBox.classList.add('show');responseBox.classList.remove('success','error');responseBody.textContent='Sending request...';responseStatus.textContent='';try{const options={method,headers};if(body&&method!=='GET'&&method!=='DELETE'){options.body=body;}const start=Date.now();const response=await fetch(url,options);const duration=Date.now()-start;const text=await response.text();responseBox.classList.add(response.ok?'success':'error');responseStatus.innerHTML=`<span style=\"color:${response.ok?'#28a745':'#dc3545'}\">Status: ${response.status} ${response.statusText}</span> | Time: ${duration}ms`;try{const json=JSON.parse(text);responseBody.textContent=JSON.stringify(json,null,2);}catch{responseBody.textContent=text;}}catch(err){responseBox.classList.add('error');responseStatus.textContent='Error';responseBody.textContent='Error: '+err.message;}}</script></body></html>";
            crest_response_html(&res, 200, playground_html);
        }
    }
    else {
        // Find matching route (thread-safe read)
        bool found = false;
        std::lock_guard<std::mutex> lock(*static_cast<std::mutex*>(app->route_mutex));
        for (size_t i = 0; i < app->route_count; i++) {
            const char* method_str = "";
            switch (app->routes[i].method) {
                case CREST_GET: method_str = "GET"; break;
                case CREST_POST: method_str = "POST"; break;
                case CREST_PUT: method_str = "PUT"; break;
                case CREST_DELETE: method_str = "DELETE"; break;
                case CREST_PATCH: method_str = "PATCH"; break;
                default: break;
            }
            
            if (strcmp(req.method, method_str) == 0 && strcmp(req.path, app->routes[i].path) == 0) {
                found = true;
                if (app->routes[i].cpp_handler) {
                    // Call C++ handler
                    auto* handler = static_cast<crest::Handler*>(app->routes[i].cpp_handler);
                    crest::Request cpp_req(&req);
                    crest::Response cpp_res(&res);
                    (*handler)(cpp_req, cpp_res);
                } else if (app->routes[i].handler) {
                    // Call C handler
                    app->routes[i].handler(&req, &res);
                }
                break;
            }
        }
        
        if (!found) {
            crest_response_json(&res, 404, "{\"error\":\"Not Found\"}");
        }
    }
    
    // Log request
    crest_log_request(req.method, req.path, res.status);
    
    if (res.body) {
        send(client_socket, res.body, (int)strlen(res.body), 0);
        free(res.body);
    }
    
    free(req.method);
    free(req.path);
    free(req.body);
    
    closesocket(client_socket);
}

static void parse_request(const char* buffer, crest_request_t* req) {
    char method[16] = {0};
    char path[1024] = {0};
    
    sscanf(buffer, "%15s %1023s", method, path);
    
    req->method = strdup(method);
    req->path = strdup(path);
    req->body = strdup("");
    
    const char* body_start = strstr(buffer, "\r\n\r\n");
    if (body_start) {
        free(req->body);
        req->body = strdup(body_start + 4);
    }
}

static const char* get_swagger_html(crest_app_t* app) {
    static char html[65536];
    
    if (app->route_count == 0) {
        snprintf(html, sizeof(html),
            "<!DOCTYPE html><html><head><meta charset='utf-8'><title>%s</title>"
            "<meta name='viewport' content='width=device-width,initial-scale=1'>"
            "<style>body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;margin:0;background:#fafafa}"
            ".header{background:linear-gradient(135deg,#667eea 0%%,#764ba2 100%%);color:white;padding:40px 20px;position:relative}"
            ".refresh-btn{position:absolute;top:20px;right:20px;background:rgba(255,255,255,0.2);border:2px solid white;color:white;padding:10px 20px;border-radius:6px;cursor:pointer;font-size:14px;transition:all 0.3s}"
            ".refresh-btn:hover{background:rgba(255,255,255,0.3);transform:scale(1.05)}"
            ".container{max-width:1200px;margin:40px auto;padding:20px;background:white;border-radius:8px;box-shadow:0 2px 8px rgba(0,0,0,0.1)}"
            "h1{font-size:2.5em;margin-bottom:10px}p{color:#666;margin:10px 0}</style></head>"
            "<body><div class='header'><button class='refresh-btn' onclick='location.reload()'>🔄 Refresh</button>"
            "<h1>%s</h1><p>%s</p><p><strong>Version:</strong> %s</p></div>"
            "<div class='container'><h2>⚠️ No Routes Defined</h2>"
            "<p>Add routes to your API to see them documented here.</p></div></body></html>",
            app->title, app->title, app->description, app->version);
        return html;
    }
    
    char routes_html[32768] = {0};
    for (size_t i = 0; i < app->route_count; i++) {
        const char* method_str = "";
        const char* method_color = "";
        const char* req_body = "";
        const char* res_body = "";
        
        switch (app->routes[i].method) {
            case CREST_GET: 
                method_str = "GET"; method_color = "#61affe";
                req_body = "None";
                res_body = "{&quot;data&quot;: &quot;string&quot;}";
                break;
            case CREST_POST: 
                method_str = "POST"; method_color = "#49cc90";
                req_body = "{&quot;name&quot;: &quot;string&quot;, &quot;value&quot;: &quot;string&quot;}";
                res_body = "{&quot;id&quot;: &quot;number&quot;, &quot;status&quot;: &quot;string&quot;}";
                break;
            case CREST_PUT: 
                method_str = "PUT"; method_color = "#fca130";
                req_body = "{&quot;name&quot;: &quot;string&quot;, &quot;value&quot;: &quot;string&quot;}";
                res_body = "{&quot;status&quot;: &quot;string&quot;}";
                break;
            case CREST_DELETE: 
                method_str = "DELETE"; method_color = "#f93e3e";
                req_body = "None";
                res_body = "{&quot;status&quot;: &quot;string&quot;}";
                break;
            case CREST_PATCH: 
                method_str = "PATCH"; method_color = "#50e3c2";
                req_body = "{&quot;field&quot;: &quot;string&quot;}";
                res_body = "{&quot;status&quot;: &quot;string&quot;}";
                break;
            default: 
                method_str = "UNKNOWN"; method_color = "#999";
                req_body = "Unknown";
                res_body = "Unknown";
                break;
        }
        
        // Use custom schemas if set, otherwise use defaults
        if (app->routes[i].request_schema) {
            req_body = app->routes[i].request_schema;
        }
        if (app->routes[i].response_schema) {
            res_body = app->routes[i].response_schema;
        }
        // Note: Schemas will be auto-detected on first request
        
        char route_html[4096];
        snprintf(route_html, sizeof(route_html),
            "<div class='endpoint'>"
            "<div class='endpoint-header' onclick='toggleEndpoint(%zu)'>"
            "<span class='method' style='background:%s'>%s</span>"
            "<span class='path'>%s</span>"
            "<span class='toggle'>▼</span>"
            "</div>"
            "<div class='endpoint-body' id='endpoint-%zu' style='display:none'>"
            "<div class='description'>%s</div>"
            "<div class='section'><h4>📥 Request Schema</h4>"
            "<div class='schema-box'><pre>%s</pre></div></div>"
            "<div class='section'><h4>📤 Response Schema (200 OK)</h4>"
            "<div class='schema-box success'><pre>%s</pre></div></div>"
            "<div class='section'><h4>📊 Possible Responses</h4>"
            "<div class='response-list'>"
            "<div class='response-item'><span class='status-code success'>200</span> Success</div>"
            "<div class='response-item'><span class='status-code error'>400</span> Bad Request</div>"
            "<div class='response-item'><span class='status-code error'>404</span> Not Found</div>"
            "<div class='response-item'><span class='status-code error'>500</span> Internal Server Error</div>"
            "</div></div>"
            "<div class='section'><h4>🚀 Try it out</h4>"
            "<button class='try-btn' onclick='tryEndpoint(\"%s\", \"%s\", %zu)'>Execute Request</button>"
            "<div class='result' id='result-%zu'></div>"
            "</div></div></div>",
            i, method_color, method_str, app->routes[i].path, i,
            app->routes[i].description[0] ? app->routes[i].description : "No description provided",
            req_body, res_body, method_str, app->routes[i].path, i, i);
        strcat(routes_html, route_html);
    }
    
    snprintf(html, sizeof(html),
        "<!DOCTYPE html><html><head><meta charset='utf-8'><title>%s - API Documentation</title>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<style>*{margin:0;padding:0;box-sizing:border-box}"
        "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,Oxygen,Ubuntu,sans-serif;background:#fafafa;color:#333}"
        ".header{background:linear-gradient(135deg,#667eea 0%%,#764ba2 100%%);color:white;padding:40px 20px;position:relative;box-shadow:0 4px 6px rgba(0,0,0,0.1)}"
        ".header h1{font-size:2.5em;margin-bottom:10px;font-weight:600}.header p{font-size:1.1em;opacity:0.95;margin:5px 0}"
        ".refresh-btn{position:absolute;top:20px;right:20px;background:rgba(255,255,255,0.2);border:2px solid white;color:white;padding:10px 20px;border-radius:6px;cursor:pointer;font-size:14px;font-weight:600;transition:all 0.3s}"
        ".refresh-btn:hover{background:rgba(255,255,255,0.3);transform:scale(1.05)}"
        ".container{max-width:1200px;margin:0 auto;padding:20px}"
        ".info{background:white;padding:25px;margin:20px 0;border-radius:8px;box-shadow:0 2px 8px rgba(0,0,0,0.08)}"
        ".info h2{color:#667eea;margin-bottom:15px;font-size:1.5em}.info p{margin:8px 0;font-size:1.05em}"
        ".info a{color:#667eea;text-decoration:none;font-weight:600}.info a:hover{text-decoration:underline}"
        ".endpoints{background:white;padding:20px;border-radius:8px;box-shadow:0 2px 8px rgba(0,0,0,0.08)}"
        ".endpoint{margin:15px 0;border:1px solid #e0e0e0;border-radius:8px;overflow:hidden;transition:all 0.3s}"
        ".endpoint:hover{box-shadow:0 4px 12px rgba(0,0,0,0.1)}"
        ".endpoint-header{padding:15px 20px;background:#f8f9fa;cursor:pointer;display:flex;align-items:center;transition:background 0.3s}"
        ".endpoint-header:hover{background:#e9ecef}"
        ".method{display:inline-block;padding:6px 14px;border-radius:4px;color:white;font-weight:700;margin-right:15px;font-size:0.85em;text-transform:uppercase;letter-spacing:0.5px}"
        ".path{font-size:1.15em;font-weight:500;color:#333;flex:1;font-family:'Courier New',monospace}"
        ".toggle{font-size:1.2em;color:#666;transition:transform 0.3s}.toggle.open{transform:rotate(180deg)}"
        ".endpoint-body{padding:20px;background:white;border-top:1px solid #e0e0e0}"
        ".description{padding:15px;background:#f8f9fa;border-left:4px solid #667eea;margin-bottom:20px;border-radius:4px;font-size:1.05em}"
        ".section{margin:20px 0}.section h4{color:#667eea;margin-bottom:12px;font-size:1.1em;font-weight:600}"
        ".schema-box{background:#f8f9fa;border:1px solid #e0e0e0;border-radius:6px;padding:15px;font-family:'Courier New',monospace;font-size:0.95em;overflow-x:auto}"
        ".schema-box.success{border-left:4px solid #49cc90}.schema-box pre{margin:0;white-space:pre-wrap;word-wrap:break-word}"
        ".response-list{display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));gap:10px}"
        ".response-item{padding:12px;background:#f8f9fa;border-radius:6px;display:flex;align-items:center;font-size:0.95em}"
        ".status-code{display:inline-block;padding:4px 10px;border-radius:4px;font-weight:700;margin-right:10px;font-size:0.9em}"
        ".status-code.success{background:#d4edda;color:#155724}.status-code.error{background:#f8d7da;color:#721c24}"
        ".try-btn{background:#667eea;color:white;border:none;padding:12px 24px;border-radius:6px;cursor:pointer;font-size:1em;font-weight:600;transition:all 0.3s}"
        ".try-btn:hover{background:#5568d3;transform:translateY(-2px);box-shadow:0 4px 8px rgba(102,126,234,0.3)}"
        ".result{margin-top:15px;padding:15px;background:#f8f9fa;border-radius:6px;font-family:'Courier New',monospace;font-size:0.9em;display:none}"
        ".result.show{display:block}.result.success{border-left:4px solid #49cc90}.result.error{border-left:4px solid #f93e3e}"
        "@media(max-width:768px){.header h1{font-size:1.8em}.container{padding:10px}.refresh-btn{top:10px;right:10px;padding:8px 16px;font-size:12px}"
        ".endpoint-header{flex-direction:column;align-items:flex-start}.method{margin-bottom:8px}.path{font-size:1em}}"
        "</style>"
        "<script>"
        "function toggleEndpoint(id){var el=document.getElementById('endpoint-'+id);var toggle=event.currentTarget.querySelector('.toggle');"
        "if(el.style.display==='none'){el.style.display='block';toggle.classList.add('open');}else{el.style.display='none';toggle.classList.remove('open');}}"
        "function tryEndpoint(method,path,id){var resultEl=document.getElementById('result-'+id);"
        "resultEl.className='result show';resultEl.innerHTML='<strong>Sending '+method+' request to '+path+'...</strong>';"
        "fetch(path,{method:method}).then(r=>r.text()).then(data=>{resultEl.className='result show success';"
        "resultEl.innerHTML='<strong>Response ('+method+' '+path+'):</strong><br><br>'+data;}).catch(err=>{"
        "resultEl.className='result show error';resultEl.innerHTML='<strong>Error:</strong><br><br>'+err.message;});}"
        "</script></head>"
        "<body><div class='header'><button class='refresh-btn' onclick='location.reload()'>🔄 Refresh</button>"
        "<h1>%s</h1><p>%s</p><p><strong>Version:</strong> %s | <strong>Powered by:</strong> Crest %s</p></div>"
        "<div class='container'><div class='info'><h2>📚 API Documentation</h2>"
        "<p><strong>Total Endpoints:</strong> %zu</p>"
        "<p><strong>OpenAPI Specification:</strong> <a href='/openapi.json' target='_blank'>View JSON</a></p>"
        "<p><strong>Interactive Playground:</strong> <a href='/playground' target='_blank'>Test API 🎮</a></p>"
        "<p><strong>Base URL:</strong> <code>/</code></p></div>"
        "<div class='endpoints'><h2 style='margin-bottom:20px;color:#667eea'>Endpoints</h2>%s</div></div></body></html>",
        app->title, app->title, app->description, app->version, CREST_VERSION,
        app->route_count, routes_html);
    
    return html;
}

static const char* get_openapi_json(crest_app_t* app) {
    static char json[32768];
    
    char paths[24576] = {0};
    for (size_t i = 0; i < app->route_count; i++) {
        const char* method_str = "";
        const char* req_schema = "";
        const char* res_schema = "";
        
        switch (app->routes[i].method) {
            case CREST_GET: 
                method_str = "get";
                req_schema = "";
                res_schema = "{\\\"type\\\":\\\"object\\\",\\\"properties\\\":{\\\"data\\\":{\\\"type\\\":\\\"object\\\"}}}";
                break;
            case CREST_POST: 
                method_str = "post";
                req_schema = "{\\\"required\\\":true,\\\"content\\\":{\\\"application/json\\\":{\\\"schema\\\":{\\\"type\\\":\\\"object\\\",\\\"properties\\\":{\\\"name\\\":{\\\"type\\\":\\\"string\\\"},\\\"value\\\":{\\\"type\\\":\\\"string\\\"}}}}}}";
                res_schema = "{\\\"type\\\":\\\"object\\\",\\\"properties\\\":{\\\"id\\\":{\\\"type\\\":\\\"integer\\\"},\\\"status\\\":{\\\"type\\\":\\\"string\\\"}}}";
                break;
            case CREST_PUT: 
                method_str = "put";
                req_schema = "{\\\"required\\\":true,\\\"content\\\":{\\\"application/json\\\":{\\\"schema\\\":{\\\"type\\\":\\\"object\\\",\\\"properties\\\":{\\\"name\\\":{\\\"type\\\":\\\"string\\\"},\\\"value\\\":{\\\"type\\\":\\\"string\\\"}}}}}}";
                res_schema = "{\\\"type\\\":\\\"object\\\",\\\"properties\\\":{\\\"status\\\":{\\\"type\\\":\\\"string\\\"}}}";
                break;
            case CREST_DELETE: 
                method_str = "delete";
                req_schema = "";
                res_schema = "{\\\"type\\\":\\\"object\\\",\\\"properties\\\":{\\\"status\\\":{\\\"type\\\":\\\"string\\\"}}}";
                break;
            case CREST_PATCH: 
                method_str = "patch";
                req_schema = "{\\\"required\\\":true,\\\"content\\\":{\\\"application/json\\\":{\\\"schema\\\":{\\\"type\\\":\\\"object\\\",\\\"properties\\\":{\\\"field\\\":{\\\"type\\\":\\\"string\\\"}}}}}}";
                res_schema = "{\\\"type\\\":\\\"object\\\",\\\"properties\\\":{\\\"status\\\":{\\\"type\\\":\\\"string\\\"}}}";
                break;
            default: 
                method_str = "get";
                req_schema = "";
                res_schema = "{\\\"type\\\":\\\"object\\\"}";
                break;
        }
        
        char path_entry[2048];
        if (req_schema[0]) {
            snprintf(path_entry, sizeof(path_entry),
                "%s\\\"%s\\\":{\\\"%s\\\":{\\\"summary\\\":\\\"%s\\\",\\\"description\\\":\\\"%s\\\",\\\"requestBody\\\":%s,"
                "\\\"responses\\\":{\\\"200\\\":{\\\"description\\\":\\\"Successful response\\\",\\\"content\\\":{\\\"application/json\\\":{\\\"schema\\\":%s}}},"
                "\\\"400\\\":{\\\"description\\\":\\\"Bad Request\\\"},\\\"404\\\":{\\\"description\\\":\\\"Not Found\\\"},\\\"500\\\":{\\\"description\\\":\\\"Internal Server Error\\\"}}}}",
                i > 0 ? "," : "", app->routes[i].path, method_str,
                app->routes[i].description[0] ? app->routes[i].description : "No description",
                app->routes[i].description[0] ? app->routes[i].description : "No description",
                req_schema, res_schema);
        } else {
            snprintf(path_entry, sizeof(path_entry),
                "%s\\\"%s\\\":{\\\"%s\\\":{\\\"summary\\\":\\\"%s\\\",\\\"description\\\":\\\"%s\\\","
                "\\\"responses\\\":{\\\"200\\\":{\\\"description\\\":\\\"Successful response\\\",\\\"content\\\":{\\\"application/json\\\":{\\\"schema\\\":%s}}},"
                "\\\"400\\\":{\\\"description\\\":\\\"Bad Request\\\"},\\\"404\\\":{\\\"description\\\":\\\"Not Found\\\"},\\\"500\\\":{\\\"description\\\":\\\"Internal Server Error\\\"}}}}",
                i > 0 ? "," : "", app->routes[i].path, method_str,
                app->routes[i].description[0] ? app->routes[i].description : "No description",
                app->routes[i].description[0] ? app->routes[i].description : "No description",
                res_schema);
        }
        strcat(paths, path_entry);
    }
    
    snprintf(json, sizeof(json),
        "{\\\"openapi\\\":\\\"3.0.0\\\",\\\"info\\\":{\\\"title\\\":\\\"%s\\\",\\\"description\\\":\\\"%s\\\",\\\"version\\\":\\\"%s\\\","
        "\\\"contact\\\":{\\\"name\\\":\\\"API Support\\\",\\\"email\\\":\\\"contact@muhammadfiaz.com\\\"}},"
        "\\\"servers\\\":[{\\\"url\\\":\\\"/\\\",\\\"description\\\":\\\"Current server\\\"}],"
        "\\\"paths\\\":{%s},"
        "\\\"components\\\":{\\\"schemas\\\":{\\\"Error\\\":{\\\"type\\\":\\\"object\\\",\\\"properties\\\":{\\\"error\\\":{\\\"type\\\":\\\"string\\\"}}}}}}",
        app->title, app->description, app->version, paths);
    
    return json;
}
