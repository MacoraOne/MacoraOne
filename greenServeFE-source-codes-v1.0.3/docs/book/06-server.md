# 6. Server Programming

The distributed server module provides HTTP server and HTTP client functionality.

## Basic server

~~~gsve
import server;

func home(req, res) {
    res.send("Hello from greenServeFE");
}

server.get("/", home);

server.serve(".", 8080, {
    index: "index.html",
    static: true
});
~~~

The current serve signature is server.serve(root, port, options).

Routes include get, post, put, patch, delete, options, any, use, and clear_routes.

Request fields include method, path, queryString, query, body, headers, and params.

Response methods include status, set, type, send, json, html, redirect, sendFile, and render.

A route handler is a callable GSVE function. The VM/native callback bridge invokes it from native HTTP handling.
