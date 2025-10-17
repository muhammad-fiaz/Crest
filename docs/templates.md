# Template Engine

Built-in template engine for rendering dynamic HTML pages.

## Overview

Features:
- Variable substitution
- Conditionals
- Loops
- Filters
- Template caching
- HTML auto-escaping

## Basic Usage

```cpp
#include "crest/crest.hpp"
#include "crest/template.hpp"

int main() {
    crest::App app;
    
    crest::TemplateEngine::Config config;
    config.template_dir = "./templates";
    config.extension = ".html";
    config.cache_enabled = true;
    config.auto_escape = true;
    
    crest::TemplateEngine engine(config);
    
    app.get("/", [&engine](crest::Request& req, crest::Response& res) {
        crest::TemplateContext context = {
            {"title", "Home Page"},
            {"username", "Alice"},
            {"logged_in", true}
        };
        
        std::string html = engine.render("index", context);
        res.html(200, html);
    });
    
    app.run("0.0.0.0", 8000);
    return 0;
}
```

## Template Syntax

### Variables

```html
<h1>{{ title }}</h1>
<p>Welcome, {{ username }}!</p>
```

### Conditionals

```html
{% if logged_in %}
    <p>You are logged in</p>
{% endif %}

{% if is_admin %}
    <a href="/admin">Admin Panel</a>
{% endif %}
```

### Filters

```html
<h1>{{ title|upper }}</h1>
<p>{{ username|capitalize }}</p>
<span>{{ email|lower }}</span>
```

## Built-in Filters

- `upper` - Convert to uppercase
- `lower` - Convert to lowercase
- `capitalize` - Capitalize first letter

## Custom Filters

```cpp
engine.add_filter("reverse", [](const std::string& s) {
    return std::string(s.rbegin(), s.rend());
});

engine.add_filter("truncate", [](const std::string& s) {
    return s.length() > 50 ? s.substr(0, 50) + "..." : s;
});
```

Usage in template:
```html
<p>{{ text|reverse }}</p>
<p>{{ description|truncate }}</p>
```

## Global Variables

```cpp
engine.add_global("site_name", "My Website");
engine.add_global("version", "1.0.0");
engine.add_global("year", 2024);
```

Access in any template:
```html
<footer>
    <p>&copy; {{ year }} {{ site_name }} v{{ version }}</p>
</footer>
```

## Template Inheritance

### Base Template (base.html)

```html
<!DOCTYPE html>
<html>
<head>
    <title>{{ title }} - {{ site_name }}</title>
</head>
<body>
    <header>
        <h1>{{ site_name }}</h1>
    </header>
    
    <main>
        <!-- Content goes here -->
    </main>
    
    <footer>
        <p>&copy; {{ year }} {{ site_name }}</p>
    </footer>
</body>
</html>
```

### Child Template (page.html)

```html
<h2>{{ page_title }}</h2>
<p>{{ content }}</p>
```

## Render String

```cpp
std::string template_str = "<h1>{{ title }}</h1><p>{{ message }}</p>";

crest::TemplateContext context = {
    {"title", "Hello"},
    {"message", "Welcome to Crest!"}
};

std::string html = engine.render_string(template_str, context);
```

## Complete Example

### Template (profile.html)

```html
<!DOCTYPE html>
<html>
<head>
    <title>{{ username }}'s Profile</title>
</head>
<body>
    <h1>{{ username|capitalize }}</h1>
    
    {% if is_verified %}
        <span class="badge">Verified</span>
    {% endif %}
    
    <div class="info">
        <p>Email: {{ email|lower }}</p>
        <p>Member since: {{ join_date }}</p>
    </div>
    
    {% if is_admin %}
        <div class="admin-panel">
            <h2>Admin Controls</h2>
            <button>Manage Users</button>
        </div>
    {% endif %}
</body>
</html>
```

### Application Code

```cpp
#include "crest/crest.hpp"
#include "crest/template.hpp"

int main() {
    crest::App app;
    crest::TemplateEngine engine;
    
    engine.add_global("site_name", "My App");
    engine.add_global("year", 2024);
    
    app.get("/profile/:username", [&engine](crest::Request& req, crest::Response& res) {
        // Get user data (simplified)
        crest::TemplateContext context = {
            {"username", "alice"},
            {"email", "alice@example.com"},
            {"join_date", "2024-01-15"},
            {"is_verified", true},
            {"is_admin", false}
        };
        
        std::string html = engine.render("profile", context);
        res.html(200, html);
    });
    
    app.run("0.0.0.0", 8000);
    return 0;
}
```

## Blog Example

```cpp
#include "crest/crest.hpp"
#include "crest/template.hpp"

struct Post {
    std::string title;
    std::string content;
    std::string author;
    std::string date;
};

int main() {
    crest::App app;
    crest::TemplateEngine engine;
    
    std::vector<Post> posts = {
        {"First Post", "Content here", "Alice", "2024-01-01"},
        {"Second Post", "More content", "Bob", "2024-01-02"}
    };
    
    app.get("/blog", [&engine, &posts](crest::Request& req, crest::Response& res) {
        // Render blog list
        std::string html = "<h1>Blog Posts</h1>";
        
        for (const auto& post : posts) {
            crest::TemplateContext context = {
                {"title", post.title},
                {"author", post.author},
                {"date", post.date}
            };
            
            html += engine.render_string(
                "<article>"
                "<h2>{{ title }}</h2>"
                "<p>By {{ author }} on {{ date }}</p>"
                "</article>",
                context
            );
        }
        
        res.html(200, html);
    });
    
    app.run("0.0.0.0", 8000);
    return 0;
}
```

## Cache Management

```cpp
// Clear template cache
engine.clear_cache();

// Disable caching for development
crest::TemplateEngine::Config config;
config.cache_enabled = false;
crest::TemplateEngine engine(config);
```

## HTML Escaping

Auto-escaping is enabled by default to prevent XSS attacks:

```cpp
crest::TemplateContext context = {
    {"user_input", "<script>alert('XSS')</script>"}
};

// Renders as: &lt;script&gt;alert('XSS')&lt;/script&gt;
```

Disable for trusted content:
```cpp
config.auto_escape = false;
```

## Best Practices

- Enable caching in production
- Use auto-escaping for user input
- Keep templates in separate directory
- Use template inheritance for consistency
- Add global variables for common data
- Create custom filters for reusable logic
- Clear cache when templates change
- Validate template context data
- Handle missing templates gracefully
- Use meaningful variable names
