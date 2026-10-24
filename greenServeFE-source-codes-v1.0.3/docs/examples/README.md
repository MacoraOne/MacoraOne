# Examples

The examples are ordered from basic language syntax to complete server patterns.

Examples 01 through 25 use only the core executable and are run by CI.

Examples 26 through 28 demonstrate separately distributed native packages. Install the corresponding shared libraries and set GREENSERVE_MODULE_PATH before running them.

Examples 29 through 34 demonstrate the server package. Run them from a directory containing the required template files.

The server examples intentionally perform API work on the server side. They do not require browser JavaScript fetch calls.

For the rendering example:

~~~text
cd docs/examples
greenServeFE 33-server-render.gsve
~~~

Then open the configured local server address.
