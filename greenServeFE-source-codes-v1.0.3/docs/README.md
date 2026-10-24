# greenServeFE Documentation

This directory is the complete learning and reference documentation for greenServeFE. It is deliberately organized as a book plus a reference manual and executable examples.

## Documentation set

The book explains the language from first installation through values, arrays, objects, control flow, functions, closures, modules, native packages, HTTP servers, templates, bytecode, the VM, debugging, native development, and project organization.

The reference directory gives concise API-oriented information for the language, built-ins, server package, gsnum, gs_table, bytecode, packaging, and the native ABI.

The examples directory contains programs that progress from single expressions to data processing and server applications.

## Distribution

Public distribution repository:

https://github.com/dominexmacedon-docs/greenServeFE-

Source and build repository:

https://github.com/dominexmacedon-docs/greenServeFE-build

Documentation must be kept synchronized with the implementation and native package ABI.

## Building the book

The GitHub Actions workflow:

1. builds greenServeFE;
2. runs the core language examples;
3. checks that the documentation tree is complete;
4. assembles the numbered book chapters;
5. builds HTML with Pandoc;
6. builds PDF with Pandoc and XeLaTeX;
7. uploads the generated book files as an artifact.

Native package examples are documented but are not claimed as CI-tested unless the corresponding shared libraries are actually installed in the job.

## Recommended reading

Read the book in numerical order. Use the reference directory when you already know the language and need an API detail.
