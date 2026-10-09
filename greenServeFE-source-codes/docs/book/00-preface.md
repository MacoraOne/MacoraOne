# greenServeFE Book

## Preface

greenServeFE is the fast-execution implementation of the greenServe language. The implementation combines a lexer, parser, compiler, bytecode representation, runtime values, environments, and a virtual machine. Native shared libraries extend the runtime without putting every facility into the core executable.

This book is written for two groups. The first is a programmer who wants to write greenServe programs. The second is a developer who wants to understand how the implementation turns a source file into executable behavior.

The book follows the current implementation. It does not treat syntax from another language as if greenServeFE already supported it.

## The learning method

Every important idea is paired with source code. Save the examples as .gsve files and run them. Then modify them. Change constants, add branches, move code into functions, and inspect the result with the diagnostic modes.

A programming language is easier to understand when its behavior is observed rather than only described.

## Implementation map

A source file travels through these stages:

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
GSCode bytecode
  |
virtual machine
  |
runtime values and environments
~~~

Native modules connect to the VM through a defined API. The server package additionally uses the callback bridge to invoke GSVE functions as HTTP route handlers.

## Book roadmap

Chapter 1 installs and runs the implementation.

Chapters 2 through 5 teach source syntax, values, data structures, control flow, and functions.

Chapters 6 through 10 explain modules, HTTP, templates, gsnum, and gs_table.

Chapters 11 through 15 explain bytecode, project design, debugging, native package development, and the complete language reference.

The result is intended to be a real distribution document rather than a short README.
