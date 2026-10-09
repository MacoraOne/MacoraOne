# 10. Debugging and Maintenance

Build the interpreter with:

~~~bash
make clean all
~~~

Run:

~~~bash
./greenServeFE test.gsve
~~~

Inspect bytecode:

~~~bash
./greenServeFE --dump test.gsve
./greenServeFE --trace test.gsve
~~~

For a native module failure, verify the shared object, initializer symbol, ABI version, module path, import name, and initializer return value.

When changing language or VM behavior, update implementation, tests, executable examples, reference material, and the generated book together.
