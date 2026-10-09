# greenServeFE

**greenServeFE** (greenServe Fast Execution) is the VM-based execution engine for the greenServe language.

It keeps the greenServe lexer/parser language model but replaces recursive AST evaluation with compiled bytecode and an explicit virtual machine.

## Architecture

```
.gsve
  -> lexer
  -> parser / AST
  -> compiler
  -> GSCode bytecode
  -> VM
  -> values / environments / native functions
```

The execution architecture is inspired by ideas visible in CPython's current source—code objects, instruction streams, constants and names tables, instruction pointers, operand stacks, frames and opcode dispatch. It is an independent greenServe implementation and does not copy CPython implementation code.

The current greenSERVE repository uses the simpler lexer -> parser -> recursive evaluator architecture. greenServeFE keeps that frontend language structure while moving execution into a bytecode compiler and VM.

## VM

A `GSCode` contains instructions, constants, names, child function code objects and parameter metadata.

Each instruction has an opcode, integer argument and source line:

```c
typedef struct {
    uint8_t op;
    int32_t arg;
    int line;
} GSInstr;
```

Frames keep an instruction pointer, code object, operand-stack base and lexical environment.

The VM currently supports:

- numbers, strings, booleans and null;
- arrays and objects;
- arithmetic and comparisons;
- logical operators;
- assignment, indexing and members;
- if/else;
- while;
- for/in;
- break and continue;
- functions, recursion and captured environments;
- native functions;
- import/from-import for the built-in module layer;
- bytecode disassembly and execution tracing.

Built-ins are `print`, `show`, `len`, `str`, `num` and `type`.

The `server` import currently exposes the VM-native print/show surface. The existing greenSERVE Node.js HTTP/server bridge is deliberately kept as a separate host-runtime integration step rather than falsely claiming that its full networking implementation has already been ported to the new VM ABI.

## Build

```text
make clean all
./greenServeFE test.gsve
./greenServeFE --dump test.gsve
./greenServeFE --trace test.gsve
```

The included test exercises bytecode execution through iteration, arithmetic, a function call, arrays, objects and conditional execution.
