# Debugging

## Lexer errors

A lexer error reports a line and column. Check the character at that location first.

## Parse errors

Typical causes include missing semicolons, parentheses, braces, or brackets, malformed function parameters, and malformed object entries.

Reduce a failing file until the smallest accepted program is found.

## Compile errors

The compiler validates constructs such as break and continue. These statements must appear inside loops.

Example of an invalid construct:

~~~gsve
break;
~~~

## Runtime errors

Runtime errors can report the code name and line. Common categories include undefined names, non-callable values, invalid iterators, stack failures, and module-load failures.

A small diagnostic program:

~~~gsve
define value = 10;
print(type(value));
print(str(value));
print(num(value));
~~~

## Module debugging

If a module cannot load, check:

~~~text
echo "$GREENSERVE_MODULE_PATH"
ls "$GREENSERVE_MODULE_PATH"
~~~

Then confirm that the .so file exists and exports the required initializer.

## Server debugging

Debug a server in layers:

1. start with one route;
2. verify the native server module;
3. test the response;
4. add request parameters;
5. add templates;
6. add static files;
7. add external HTTP calls.

Do not debug all layers simultaneously.

## Trace mode

Trace mode shows code name, instruction offset, operation name, and argument. It is useful for checking variable access, calls, returns, loops, indexing, and imports.

## Reproduction discipline

When reporting an implementation bug, provide:

- greenServeFE version;
- operating system;
- source file;
- exact command;
- expected behavior;
- actual output;
- whether native modules are involved;
- dump or trace output when relevant.

A minimal reproduction is usually much easier to fix than a complete application.
