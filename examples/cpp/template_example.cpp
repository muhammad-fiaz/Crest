/**
 * @file template_example.cpp
 * @brief Template engine example
 */

#include "crest/crest.hpp"
#include "crest/template.hpp"
#include <iostream>
#include <filesystem>

int main() {
    crest::App app;
    
    // Create templates directory
    std::filesystem::create_directories("./templates");
    
    // Configure template engine
    crest::TemplateEngine::Config config;
    config.template_dir = "./templates";
    config.extension = ".html";
    config.cache_enabled = true;
    config.auto_escape = true;
    
    crest::TemplateEngine engine(config);
    
    // Add global variables
    engine.add_global("site_name", "Crest Demo");
    engine.add_global("version", "1.0.0");
    engine.add_global("year", 2024);
    
    // Add custom filters
    engine.add_filter("reverse", [](const std::string& s) {
        return std::string(s.rbegin(), s.rend());
    });
    
    engine.add_filter("truncate", [](const std::string& s) {
        return s.length() > 50 ? s.substr(0, 50) + "..." : s;
    });
    
    // Home page
    app.get("/", [&engine](crest::Request& req, crest::Response& res) {
        crest::TemplateContext context = {
            {"title", "Home"},
            {"heading", "Welcome to Crest"},
            {"message", "A modern RESTful API framework for C++"}
        };
        
        std::string html = engine.render_string(R"(
<!DOCTYPE html>
<html>
<head>
    <title>{{ title }} - {{ site_name }}</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 800px; margin: 50px auto; }
        h1 { color: #007bff; }
        .footer { margin-top: 50px; padding-top: 20px; border-top: 1px solid #ccc; color: #666; }
    </style>
</head>
<body>
    <h1>{{ heading }}</h1>
    <p>{{ message }}</p>
    <p><a href="/profile">View Profile</a> | <a href="/blog">Blog</a> | <a href="/about">About</a></p>
    <div class="footer">
        <p>&copy; {{ year }} {{ site_name }} v{{ version }}</p>
    </div>
</body>
</html>
        )", context);
        
        res.html(200, html);
    });
    
    // Profile page with conditionals
    app.get("/profile", [&engine](crest::Request& req, crest::Response& res) {
        crest::TemplateContext context = {
            {"title", "Profile"},
            {"username", "alice"},
            {"email", "alice@example.com"},
            {"is_verified", true},
            {"is_admin", false},
            {"member_since", "2024-01-15"}
        };
        
        std::string html = engine.render_string(R"(
<!DOCTYPE html>
<html>
<head>
    <title>{{ title }} - {{ site_name }}</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 800px; margin: 50px auto; }
        .badge { background: #28a745; color: white; padding: 5px 10px; border-radius: 3px; }
        .info { margin: 20px 0; }
    </style>
</head>
<body>
    <h1>{{ username|capitalize }}'s Profile</h1>
    
    {% if is_verified %}
        <span class="badge">Verified</span>
    {% endif %}
    
    <div class="info">
        <p><strong>Email:</strong> {{ email|lower }}</p>
        <p><strong>Member since:</strong> {{ member_since }}</p>
    </div>
    
    {% if is_admin %}
        <div class="admin-panel">
            <h2>Admin Controls</h2>
            <button>Manage Users</button>
        </div>
    {% endif %}
    
    <p><a href="/">Back to Home</a></p>
</body>
</html>
        )", context);
        
        res.html(200, html);
    });
    
    // Blog page
    app.get("/blog", [&engine](crest::Request& req, crest::Response& res) {
        std::string html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Blog - )" + std::string("Crest Demo") + R"(</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 800px; margin: 50px auto; }
        .post { margin: 30px 0; padding: 20px; border: 1px solid #ddd; border-radius: 5px; }
        .post h2 { margin-top: 0; color: #007bff; }
        .meta { color: #666; font-size: 0.9em; }
    </style>
</head>
<body>
    <h1>Blog Posts</h1>
)";
        
        // Simulate blog posts
        struct Post {
            std::string title;
            std::string content;
            std::string author;
            std::string date;
        };
        
        std::vector<Post> posts = {
            {"Getting Started with Crest", "Learn how to build APIs with Crest framework...", "Alice", "2024-01-01"},
            {"Advanced Routing", "Explore advanced routing techniques and patterns...", "Bob", "2024-01-05"},
            {"Database Integration", "Connect your API to databases easily...", "Charlie", "2024-01-10"}
        };
        
        for (const auto& post : posts) {
            crest::TemplateContext context = {
                {"title", post.title},
                {"content", post.content},
                {"author", post.author},
                {"date", post.date}
            };
            
            html += engine.render_string(R"(
    <div class="post">
        <h2>{{ title }}</h2>
        <p class="meta">By {{ author }} on {{ date }}</p>
        <p>{{ content }}</p>
    </div>
            )", context);
        }
        
        html += R"(
    <p><a href="/">Back to Home</a></p>
</body>
</html>
        )";
        
        res.html(200, html);
    });
    
    // About page with filters
    app.get("/about", [&engine](crest::Request& req, crest::Response& res) {
        crest::TemplateContext context = {
            {"title", "About"},
            {"description", "Crest is a production-ready, high-performance RESTful API framework for C and C++. Inspired by modern API frameworks for simplicity and developer experience."},
            {"features", "High Performance, C & C++ Support, Auto Documentation, Simple API, Production Ready, Cross-Platform"}
        };
        
        std::string html = engine.render_string(R"(
<!DOCTYPE html>
<html>
<head>
    <title>{{ title }} - {{ site_name }}</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 800px; margin: 50px auto; }
        .description { line-height: 1.6; }
    </style>
</head>
<body>
    <h1>About {{ site_name }}</h1>
    <div class="description">
        <p>{{ description }}</p>
        <p><strong>Features:</strong> {{ features }}</p>
        <p><strong>Version:</strong> {{ version }}</p>
    </div>
    <p><a href="/">Back to Home</a></p>
</body>
</html>
        )", context);
        
        res.html(200, html);
    });
    
    // API endpoint that returns JSON
    app.get("/api/data", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({
            "message": "This is a JSON API endpoint",
            "data": {
                "users": 100,
                "posts": 250,
                "comments": 1500
            }
        })");
    });
    
    // Form example
    app.get("/form", [&engine](crest::Request& req, crest::Response& res) {
        crest::TemplateContext context = {
            {"title", "Contact Form"}
        };
        
        std::string html = engine.render_string(R"(
<!DOCTYPE html>
<html>
<head>
    <title>{{ title }} - {{ site_name }}</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 800px; margin: 50px auto; }
        .form-group { margin: 15px 0; }
        label { display: block; margin-bottom: 5px; font-weight: bold; }
        input, textarea { width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 3px; }
        button { padding: 10px 20px; background: #007bff; color: white; border: none; cursor: pointer; }
        button:hover { background: #0056b3; }
    </style>
</head>
<body>
    <h1>{{ title }}</h1>
    <form action="/submit" method="POST">
        <div class="form-group">
            <label>Name:</label>
            <input type="text" name="name" required />
        </div>
        <div class="form-group">
            <label>Email:</label>
            <input type="email" name="email" required />
        </div>
        <div class="form-group">
            <label>Message:</label>
            <textarea name="message" rows="5" required></textarea>
        </div>
        <button type="submit">Submit</button>
    </form>
    <p><a href="/">Back to Home</a></p>
</body>
</html>
        )", context);
        
        res.html(200, html);
    });
    
    // Handle form submission
    app.post("/submit", [&engine](crest::Request& req, crest::Response& res) {
        crest::TemplateContext context = {
            {"title", "Thank You"},
            {"message", "Your message has been received!"}
        };
        
        std::string html = engine.render_string(R"(
<!DOCTYPE html>
<html>
<head>
    <title>{{ title }} - {{ site_name }}</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 800px; margin: 50px auto; }
        .success { background: #d4edda; border: 1px solid #c3e6cb; color: #155724; padding: 15px; border-radius: 5px; }
    </style>
</head>
<body>
    <h1>{{ title }}</h1>
    <div class="success">
        <p>{{ message }}</p>
    </div>
    <p><a href="/">Back to Home</a></p>
</body>
</html>
        )", context);
        
        res.html(200, html);
    });
    
    std::cout << "Template Engine Server running on http://0.0.0.0:8000" << std::endl;
    std::cout << "Open http://localhost:8000 in your browser" << std::endl;
    std::cout << std::endl;
    std::cout << "Pages:" << std::endl;
    std::cout << "  /           - Home page" << std::endl;
    std::cout << "  /profile    - Profile with conditionals" << std::endl;
    std::cout << "  /blog       - Blog posts" << std::endl;
    std::cout << "  /about      - About page with filters" << std::endl;
    std::cout << "  /form       - Contact form" << std::endl;
    std::cout << "  /api/data   - JSON API endpoint" << std::endl;
    
    app.run("0.0.0.0", 8000);
    return 0;
}
