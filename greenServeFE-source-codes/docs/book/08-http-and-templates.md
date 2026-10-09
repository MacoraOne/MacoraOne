# HTTP Clients, Files, and Templates

## HTTP response

The HTTP client functions return an object containing fields used by the server package, including status, body, content type, error, and URL.

Example:

~~~gsve
import server;

define response = server.http_get("https://example.com/");

print(response.status);
print(response.contentType);
print(response.body);
print(response.error);
print(response.url);
~~~

## Client functions

The server package exports http_get, https_get, http_post, http_put, http_patch, http_delete, and http_options.

The exact argument contract for each operation should be checked against the version of server.so being distributed.

## Server-side fetching pattern

A useful application flow is:

~~~text
client
  |
  v
greenServeFE route
  |
  v
server-side HTTP request
  |
  v
validation and transformation
  |
  v
HTML or JSON response
~~~

This keeps server-side work on the server.

## File helpers

The module exports read_file and write_file for application-side file operations.

Keep private data outside the static document root.

## Escaping helpers

The module also provides url_encode and json_escape. These are useful when output needs URL or JSON escaping.

## Rendering

The render helper and response render method are for template-style replacement.

HTML:

~~~html
<!doctype html>
<html>
<head>
    <title>{{title}}</title>
</head>
<body>
    <h1>{{title}}</h1>
    <p>{{message}}</p>
</body>
</html>
~~~

GSVE:

~~~gsve
func home(req, res) {
    res.render("index.html", {
        title: "Dashboard",
        message: "Rendered by greenServeFE"
    });
}
~~~

## Complete static plus dynamic project

Directory:

~~~text
site/
  server.gsve
  index.html
  public/
    style.css
~~~

server.gsve:

~~~gsve
import server;

func home(req, res) {
    res.render("index.html", {
        title: "Store",
        message: "Welcome to the store"
    });
}

server.get("/", home);

server.serve(".", 8080, {
    index: "index.html",
    static: true
});
~~~

The route handles the dynamic page, while CSS and other assets can be served statically.

## Path safety

Render and static file paths must remain within the configured root. Do not build a filesystem path directly from unvalidated user input.

The server implementation performs path-safety checks, but application design should also keep public and private files separated.
