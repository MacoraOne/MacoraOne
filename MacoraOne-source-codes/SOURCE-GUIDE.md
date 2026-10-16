# MacoraOne Source Archive

This directory mirrors the current build implementation from [dominexmacedon-docs/MarocaOne-build](https://github.com/dominexmacedon-docs/MarocaOne-build).

## Build sources

- `main.c` — CLI entry point and project/component workflows.
- `module_manager.c` — dynamic release catalogue, versioned package install/list/uninstall, and greenServeFE package discovery.
- `macoraone.h` — shared declarations and version information.
- `Makefile` — C11 build, install, uninstall, and smoke-test targets.
- `.github/workflows/build.yml` — push/manual build and validation workflow.
- `README.md` — full command reference and architecture documentation.

Build with `make clean all`; test with `make test`. This archive is source code, not the installed `one` executable.

Related pages: [MacoraOne CLI guide](../macoraone.html), [complete one command reference](../one-commands.html), [greenServeFE examples](../greenservefe-examples.html), and [CMLL examples](../cmll-examples.html).
