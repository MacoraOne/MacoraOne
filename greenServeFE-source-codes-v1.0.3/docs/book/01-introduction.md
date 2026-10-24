# 1. Introduction

## Execution pipeline

~~~text
.gsve
  -> lexer
  -> parser / AST
  -> compiler
  -> GSCode bytecode
  -> VM
  -> values / environments / native functions
~~~

The current VM supports numbers, strings, booleans, null, arrays, objects, arithmetic, comparisons, logical operators, assignment, indexing, members, if/else, while, for/in, break, continue, functions, recursion, captured environments, native functions, imports, bytecode disassembly, and execution tracing.

Built-ins are print, show, len, str, num, and type.
