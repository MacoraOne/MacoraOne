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

## Native utility modules

Native modules are built as Linux x86-64 shared objects. Each module has its own manually maintained GitHub Actions workflow under `.github/workflows/`; open **Actions**, select the matching workflow, and choose **Run workflow** to build its artifact. The workflows package the `.so`, C source, and SHA-256 checksums.

| Module | Exported utilities |
|---|---|
| `gsnum` | Numerical helpers (existing module) |
| `gs_table` | Formatted tables (existing module) |
| `gs_string` | `lower`, `upper`, `trim`, `contains`, `starts_with`, `ends_with` |
| `gs_json` | `stringify` for VM values |
| `gs_path` | `join`, `basename`, `dirname`, `extension` |
| `gs_time` | `now`, `format`, `utc` |
| `gs_random` | `seed`, `integer`, `fraction` (not for cryptographic security) |
| `gs_uuid` | UUID helper (existing module) |
| `gs_validate` | Validation helpers (existing module) |
| `gs_env` | Environment variable `get`, `has`, `set`, `unset` |
| `gs_hash` | `fnv1a`, `djb2`, `same_hash` (non-cryptographic hashes) |
| `gs_base64` | `encode`, `decode` |
| `gs_math` | `abs`, `floor`, `ceil`, `round`, `sqrt`, `log`, `log10`, `exp`, `pow`, `min`, `max`, `clamp`, `pi`, `e` |
| `gs_csv` | `parse`, `stringify` for CSV text and arrays of string fields |
| `gs_url` | `encode`, `decode`, `query_encode`, `query_decode` |
| `gs_file` | `exists`, `read_text`, `write_text`, `append_text`, `size` |

Example usage (after building and installing the corresponding module where the VM can find it):

```gsve
import gs_math;
import gs_url;

print(gs_math.sqrt(81));
print(gs_url.query_encode("hello world & greenServeFE"));
```

**Security note:** `gs_file` reads and writes paths with the permissions of the process running greenServeFE. Only import it for trusted scripts; it is not a sandbox. `gs_random` and `gs_hash` are general-purpose utilities, not cryptographic security tools.

## Additional native modules

Five additional native modules are now implemented as independent C sources and have individual manually triggered GitHub Actions workflows. Their workflows build Linux x86-64 shared objects, package the source and ABI headers, and include SHA-256 checksums.

| Module | Exports | Build dependency |
|---|---|---|
| `gs_http` | `request`, `get`, `post` | libcurl |
| `gs_crypto` | `sha256`, `sha512`, `random_bytes` | OpenSSL libcrypto |
| `gs_db` | `open`, `close`, `exec`, `query`, `error` | SQLite |
| `gs_log` | `log`, `debug`, `info`, `warn`, `error` | libc |
| `gs_form` | `trim`, `required`, `is_email`, `to_number`, `to_bool`, `min_length`, `max_length`, `in_range` | libc and libm |

Open **Actions** and manually run the matching `Build gs_...` workflow to produce its ZIP artifact. The workflows build the shared objects but do not automatically install them into the VM's module directory.

### API notes

- `gs_http.request(method, url, body?, headers?)` returns an object containing `status`, `ok`, `body`, `headers`, and `error`. TLS certificate/hostname verification is enabled; redirects are limited to five, connect timeout is 10 seconds, and total timeout is 30 seconds.
- `gs_crypto.random_bytes(count)` returns cryptographically secure random bytes encoded as lowercase hexadecimal; the accepted count is 1–4096 bytes. Do not use `gs_hash` for cryptographic purposes.
- `gs_db.open(path)` returns a numeric connection handle. Use `gs_db.exec(handle, sql, params?)` and `gs_db.query(handle, sql, params?)`; `params` is an array bound to SQLite placeholders. Values returned from BLOB columns are lowercase hexadecimal strings. Close connections with `gs_db.close(handle)`.
- `gs_log` writes JSON-lines records to standard error. The optional context argument supports string, number, and boolean values; other values are emitted as `null`.
- `gs_form.to_number` and `gs_form.to_bool` return null for invalid input. Email validation is a practical format check, not proof that an address exists.

See [the native module roadmap](docs/ROADMAP.md) for dependencies, constraints, and remaining validation work.

## Build

```text
make clean all
./greenServeFE test.gsve
./greenServeFE --dump test.gsve
./greenServeFE --trace test.gsve
```

The included test exercises bytecode execution through iteration, arithmetic, a function call, arrays, objects and conditional execution.
