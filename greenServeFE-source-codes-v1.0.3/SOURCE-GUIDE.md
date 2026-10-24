# greenServeFE Source Archive

This archive tracks the greenServeFE build repository and includes the lexer, parser, bytecode compiler, VM, value system, native ABI, examples, and native-module build workflows.

## Native module sources

Module C files and their corresponding manually triggered workflows are copied from [dominexmacedon-docs/greenServeFE-build](https://github.com/dominexmacedon-docs/greenServeFE-build). Utilities include `gs_db`, `gs_crypto`, `gs_http`, `gs_form`, `gs_log`, `gs_csv`, `gs_file`, `gs_math`, `gs_url`, `gs_base64`, `gs_env`, `gs_hash`, `gs_json`, `gs_path`, `gs_random`, `gs_string`, `gs_time`, `gs_uuid`, `gs_validate`, `gsnum`, `gs_table`, and `gs_object`.

Run a source example with `./greenServeFE docs/examples/01-hello.gsve`. Use `./greenServeFE --dump file.gsve` to inspect bytecode or `./greenServeFE --trace file.gsve` to trace VM execution.

Native modules are platform-specific shared libraries. A workflow builds a module artifact; it does not itself install the module into the runtime's module search path.

Related pages: [greenServeFE guide](../greenservefe.html), [GSVE examples](../greenservefe-examples.html), and [MacoraOne one commands](../one-commands.html).
