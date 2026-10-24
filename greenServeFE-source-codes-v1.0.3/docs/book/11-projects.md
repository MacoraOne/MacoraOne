# 11. Application Structure

A small server application can use:

~~~text
my-app/
├── server.gsve
├── index.html
└── data/
    └── data.json
~~~

Server-side rendering does not require browser fetch calls.

~~~gsve
import server;

func home(req, res) {
    res.render("index.html", {
        title: "My application"
    });
}

server.get("/", home);

server.serve(".", 8080, {
    index: "index.html",
    static: true
});
~~~

Validate externally supplied input before using it in paths, templates, commands, or other sensitive operations.
