/**
 * @file advanced.c
 * @brief Advanced example showing middleware, routing, and JSON handling
 */

#include "crest/crest.h"
#include "crest/router.h"
#include "crest/middleware.h"
#include <stdio.h>
#include <string.h>

/* Custom middleware example */
bool logger_middleware(crest_request_t *req, crest_response_t *res) {
    const char *methods[] = {"GET", "POST", "PUT", "DELETE", "PATCH", "HEAD", "OPTIONS"};
    int method_idx = crest_request_method(req);
    
    crest_log(CREST_LOG_INFO, "%s %s", methods[method_idx], crest_request_path(req));
    return true; // Continue to next middleware/handler
}

/* API Handlers */
void list_products(crest_request_t *req, crest_response_t *res) {
    const char *category = crest_request_query(req, "category");
    
    char json[1024] = "{\"products\":[";
    strcat(json, "{\"id\":1,\"name\":\"Laptop\",\"price\":999.99,\"category\":\"electronics\"},");
    strcat(json, "{\"id\":2,\"name\":\"Phone\",\"price\":699.99,\"category\":\"electronics\"},");
    strcat(json, "{\"id\":3,\"name\":\"Book\",\"price\":19.99,\"category\":\"books\"}");
    strcat(json, "]}");
    
    crest_response_json(res, json);
}

void get_product(crest_request_t *req, crest_response_t *res) {
    const char *id = crest_request_param(req, "id");
    
    char json[256];
    snprintf(json, sizeof(json),
            "{\"id\":%s,\"name\":\"Sample Product\",\"price\":99.99,\"in_stock\":true}",
            id ? id : "0");
    
    crest_response_json(res, json);
}

void create_product(crest_request_t *req, crest_response_t *res) {
    const char *body = crest_request_body(req);
    
    printf("[INFO] Creating product with data: %s\n", body ? body : "(no data)");
    
    crest_response_status(res, CREST_STATUS_CREATED);
    crest_response_header(res, "Location", "/api/products/123");
    crest_response_json(res, "{\"id\":123,\"message\":\"Product created\"}");
}

void update_product(crest_request_t *req, crest_response_t *res) {
    const char *id = crest_request_param(req, "id");
    const char *body = crest_request_body(req);
    
    crest_log(CREST_LOG_INFO, "Updating product %s with data: %s", id ? id : "unknown", body ? body : "(no data)");
    
    char json[256];
    snprintf(json, sizeof(json), "{\"id\":%s,\"message\":\"Product updated\"}", id ? id : "0");
    crest_response_json(res, json);
}

void delete_product(crest_request_t *req, crest_response_t *res) {
    const char *id = crest_request_param(req, "id");
    
    crest_log(CREST_LOG_INFO, "Deleting product %s", id ? id : "unknown");
    
    crest_response_status(res, CREST_STATUS_NO_CONTENT);
    crest_response_send(res, "");
}

int main(void) {
    /* Create application */
    crest_app_t *app = crest_create();
    
    if (!app) {
        crest_log(CREST_LOG_ERROR, "Failed to create application");
        return 1;
    }
    
    /* Enable dashboard */
    crest_enable_dashboard(app, true);
    crest_set_dashboard_path(app, "/docs");
    
    /* Add global middleware */
    crest_use(app, logger_middleware);
    
    /* API Routes */
    crest_get(app, "/api/products", list_products, "List all products");
    crest_get(app, "/api/products/:id", get_product, "Get product by ID");
    crest_post(app, "/api/products", create_product, "Create a new product");
    crest_put(app, "/api/products/:id", update_product, "Update product");
    crest_delete(app, "/api/products/:id", delete_product, "Delete product");
    
    /* Health check */
    crest_get(app, "/health", NULL, "Health check");
    
    /* Start server */
    printf("\n");
    printf(">>> Advanced Crest Example - Product API\n");
    printf("=========================================\n\n");
    
    int result = crest_run(app, "localhost", 8080);
    
    crest_destroy(app);
    return result;
}
