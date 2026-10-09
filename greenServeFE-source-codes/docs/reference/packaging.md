# Packaging Reference

## Core build

~~~text
make clean all
~~~

Produces the greenServeFE executable.

## Native packages

Native packages are shared objects such as gsnum.so, gs_table.so, and server.so.

A typical module directory is:

~~~text
/usr/local/lib/modules
~~~

Set:

~~~text
export GREENSERVE_MODULE_PATH=/usr/local/lib/modules
~~~

## Generated documentation

The workflow creates:

~~~text
dist/greenServeFE-book.md
dist/greenServeFE-book.html
dist/greenServeFE-book.pdf
dist/documentation-files.txt
~~~

All are uploaded as a GitHub Actions artifact.
