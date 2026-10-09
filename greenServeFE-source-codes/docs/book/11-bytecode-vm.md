# Bytecode and the Virtual Machine

## Execution pipeline

The implementation does not execute the source syntax tree directly. The source is lexed, parsed, compiled to GSCode, and then executed by the VM.

~~~text
source
  |
lexer
  |
tokens
  |
parser
  |
program tree
  |
compiler
  |
GSCode
  |
VM
  |
runtime values
~~~

## Lexer

The lexer recognizes identifiers, numbers, strings, keywords, punctuation, and operators.

The current keyword set includes define, if, else, for, in, while, func, return, break, continue, import, from, true, false, and null.

## Parser

The parser represents expressions such as numbers, strings, arrays, objects, calls, indexes, members, assignments, unary operations, and binary operations.

It represents statements such as definitions, blocks, conditionals, loops, functions, returns, loop control, and imports.

## Compiler

The compiler creates a GSCode object containing instructions, constants, names, and child function code.

The instruction set includes load/store operations, arithmetic and comparison operations, jumps, calls, returns, array/object construction, member/index access, function creation, imports, iteration, and halt.

## Stack machine

For:

~~~gsve
define total = 2 + 3;
~~~

the compiler emits operations that load 2, load 3, add them, and store the result.

For a call, the VM places the function and arguments on the stack and creates a new frame and environment.

## Frames

A frame stores a code object, instruction pointer, stack base, and environment.

Function environments point to parent environments. A closure stores its defining environment, which is why a nested function can use an outer variable after the outer function returns.

## Native functions

A native function is a runtime value containing a C function pointer. The VM can call it directly.

Native modules receive the GSNativeAPI structure and therefore use a controlled runtime interface.

## Callback bridge

The server module needs the opposite direction: C calls a GSVE function.

The runtime therefore provides a callback function through the native API. It creates a function environment, binds parameters, pushes a VM frame, executes it, and returns its result.

## Diagnostics

Dump:

~~~text
./greenServeFE --dump example.gsve
~~~

Trace:

~~~text
./greenServeFE --trace example.gsve
~~~

Trace mode is particularly useful when debugging compiler and VM changes.
