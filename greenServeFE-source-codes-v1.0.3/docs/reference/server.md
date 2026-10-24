# Server Reference

## Module

~~~gsve
import server;
~~~

## Routes

get, post, put, patch, delete, options, any, use, clear_routes.

## Serving

~~~gsve
server.serve(".", 8080, {
    index: "index.html",
    static: true,
    cors: true,
    host: "0.0.0.0",
    backlog: 128
});
~~~

Other supported serve options include notFound, max_requests, protocol, cert, and key.

## Request

method, path, queryString, query, body, headers, params.

## Response

status, set, type, send, json, html, redirect, sendFile, render.

## HTTP client

http_get, https_get, http_post, http_put, http_patch, http_delete, http_options.

## Files and escaping

read_file, write_file, render, url_encode, json_escape.

## Route example

~~~gsve
func route(req, res) {
    res.status(200);
    res.set("X-App", "greenServeFE");
    res.json({
        method: req.method,
        path: req.path,
        query: req.query
    });
}

server.get("/api/info", route);
~~~
