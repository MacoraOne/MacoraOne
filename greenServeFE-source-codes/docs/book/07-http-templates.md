# 7. HTTP and Templates

The server module exports http_get, https_get, http_post, http_put, http_patch, http_delete, and http_options.

~~~gsve
import server;

define response = server.http_get("https://en.wikipedia.org/api/rest_v1/page/summary/Wikipedia");

show(response.status);
show(response.contentType);
show(response.body);
~~~

The request is made server-side by the native module.

## Rendering

~~~gsve
func home(req, res) {
    res.render("index.html", {
        title: "greenServeFE",
        message: "Rendered by the server"
    });
}
~~~

A template can contain placeholders such as {{title}} and {{message}}.

The server module also provides read_file, write_file, and render.
