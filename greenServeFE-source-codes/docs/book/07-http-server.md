# HTTP Server

## Basic server

The server module is a native package. Import it and register routes before serving:

~~~gsve
import server;

func home(req, res) {
    res.send("Hello World");
}

server.get("/", home);

server.serve(".", 8080, {
    index: "index.html",
    static: true
});
~~~

The first serve argument is the document root. The second is the port. The third is an options object.

## Route methods

The current route registration API includes get, post, put, patch, delete, options, any, and use.

~~~gsve
server.get("/", home);
server.post("/users", create_user);
server.put("/users/:id", replace_user);
server.patch("/users/:id", update_user);
server.delete("/users/:id", delete_user);
~~~

## Parameter routes

A colon segment creates a route parameter:

~~~gsve
func user(req, res) {
    res.json({
        id: req.params.id
    });
}

server.get("/users/:id", user);
~~~

The router decodes the parameter before exposing it.

A wildcard route uses an asterisk and exposes the matched value as req.params.wildcard.

## Prefix middleware

use registers a prefix handler:

~~~gsve
func log_request(req, res) {
    print(req.method);
    print(req.path);
}

server.use("/api", log_request);
~~~

A handler that does not end the response can perform middleware-like work.

## Request object

A handler receives:

- method
- path
- queryString
- query
- body
- headers
- params when a parameterized route matched

For a URL such as /search?q=greenServe&page=2:

~~~gsve
func search(req, res) {
    print(req.query.q);
    print(req.query.page);
    res.send("search");
}
~~~

## Response object

The response API includes:

- status
- set
- type
- send
- json
- html
- redirect
- sendFile
- render

Example:

~~~gsve
func created(req, res) {
    res.status(201);
    res.set("X-App", "greenServeFE");
    res.type("text/plain");
    res.send("created");
}
~~~

## JSON

~~~gsve
func api(req, res) {
    res.json({
        ok: true,
        name: "greenServeFE",
        items: [1, 2, 3]
    });
}
~~~

## HTML

~~~gsve
func home(req, res) {
    res.html("<h1>greenServeFE</h1>");
}
~~~

## Static files

Static serving can be enabled:

~~~gsve
server.serve(".", 8080, {
    index: "index.html",
    static: true
});
~~~

The server selects common MIME types for HTML, CSS, JavaScript, JSON, SVG, PNG, JPEG, GIF, and plain text.

## Templates

Render a file relative to the server root:

~~~gsve
func home(req, res) {
    res.render("index.html", {
        title: "Book Store",
        message: "Welcome"
    });
}
~~~

The template engine replaces placeholders in the form {{key}}. It is intentionally simple. Prepare application data in GSVE rather than expecting template code to contain loops or application logic.

## Serve options

Current options include index, static, cors, notFound, host, backlog, max_requests, protocol, cert, and key.

The default host is 0.0.0.0. The default port is 8080. The default index is index.html.

## HTTPS

TLS can be selected with certificate and key paths:

~~~gsve
server.serve(".", 8443, {
    protocol: "https",
    cert: "server.crt",
    key: "server.key",
    index: "index.html",
    static: true
});
~~~

Private keys must never be committed to source control.

## Route matching

The router prefers the most specific matching route and preserves registration order for equal specificity. HEAD requests can use matching GET routes.

## Server-side API architecture

A route can call an external API on the server:

~~~gsve
define response = server.http_get("https://example.com/");

func home(req, res) {
    res.json(response);
}
~~~

This design does not require browser fetch calls. It also lets the server transform external data before sending it to a client.
