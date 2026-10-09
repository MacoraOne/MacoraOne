# Real Project Design

## Command-line project

A small command-line project can begin as:

~~~text
calculator/
  main.gsve
~~~

As it grows, keep calculations inside functions and keep output code near the entry point.

## Server project

A practical web project can be:

~~~text
book-store/
  server.gsve
  index.html
  order.html
  data/
    books.json
  public/
    style.css
~~~

Choose the server root carefully. Private data should not be inside a directory configured for static serving.

## Server entry point

~~~gsve
import server;

func home(req, res) {
    res.render("index.html", {
        title: "Book Store"
    });
}

server.get("/", home);

server.serve(".", 8080, {
    index: "index.html",
    static: true
});
~~~

## Data processing

Represent records as objects and collections as arrays:

~~~gsve
define books = [
    {id: 1, name: "Programming", price: 30},
    {id: 2, name: "Networking", price: 25}
];

func total_for(book, quantity) {
    return book.price * quantity;
}

print(total_for(books[0], 2));
~~~

## API boundaries

Keep external API calls on the server:

~~~text
browser
  |
  v
greenServeFE route
  |
  v
external HTTP service
  |
  v
validation and transformation
  |
  v
HTML or JSON
~~~

This prevents browser source from becoming the place where server-side integration is implemented.

## Testing

Keep deterministic programs for arithmetic, arrays, functions, loops, and other core features. Run them in CI.

Native package tests should run in an environment where the exact shared libraries have been installed.

## Documentation as a release artifact

A release should contain the executable, native packages, installation instructions, API reference, examples, and generated book.

The CI workflow in this repository produces the book automatically so documentation is built from the same commit as the source.
