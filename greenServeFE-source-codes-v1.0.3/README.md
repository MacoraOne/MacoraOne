# greenServeFE 1.0.3 — Source Snapshot

This versioned folder preserves the public source archive for **greenServeFE 1.0.3**. The core lexer, parser, bytecode compiler, value model, VM, module interface, and Makefile have been synchronized from the private build repository. Selected canonical `.gsve` examples have also been refreshed.

- [Source archive index](index.html)
- [Core runtime source files](.)
- [Examples](docs/examples/)
- [Canonical private build repository](https://github.com/dominexmacedon-docs/greenServeFE-build)

## Build

Run `make clean all`, then `./greenServeFE docs/examples/01-hello.gsve`.

## Version note

The `1.0.3` label identifies this archived source snapshot. It does not itself publish or rebuild a release binary.

---

## Upstream project notes

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

Built-ins include `print`, `show`, `len`, `str`, `num`, `type`, `range`, `push`, `pop`, `keys`, `values`, and `contains`. Logical `&&` and `||` use short-circuit evaluation, so the right operand is skipped when the left operand determines the result.

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


## Native AI workspace: `gs_cortex`

`gs_cortex` is an independently imported native C module for AI-oriented work in greenServeFE. It does **not** modify the language syntax, compiler, VM, or module ABI, and it has no Python or TensorFlow runtime dependency. Import it like any other native module after building and installing `gs_cortex.so`.

The module keeps backward-compatible top-level utilities and also exposes grouped APIs:

- `gs_cortex.data`: fetch web content, extract HTML text/tags, clean text, flatten JSON text, tokenize, ingest supported files, and append JSONL dataset records.
- `gs_cortex.tensor`: inspect matrix shape, create zero matrices, add matrices, multiply matrices, transpose matrices, calculate vector dot products, and apply ReLU/sigmoid activations to numeric vectors or matrices.
- `gs_cortex.workspace()`: report the native workspace version and current capability status.
- Existing top-level `train`, `generate`, and `model_info`: a small weighted-bigram text baseline, retained as a demonstration model—not a deep-learning system.

Example greenServeFE script:

```gsve
import gs_cortex;

print(gs_cortex.workspace());

let a = [[1, 2], [3, 4]];
let b = [[5, 6], [7, 8]];
print(gs_cortex.tensor.shape(a));
print(gs_cortex.tensor.matmul(a, b));
print(gs_cortex.tensor.transpose(a));
print(gs_cortex.tensor.relu([[-2, 3], [4, -5]]));
print(gs_cortex.tensor.sigmoid([0, 1, -1]));

let page = gs_cortex.data.scrape("https://example.com");
if (page.ok) {
    print(gs_cortex.data.tokenize(page.text));
}
```

The current tensor API uses nested arrays of numbers and validates rectangular matrix shapes. Matrix operations are bounded to avoid unreasonably large allocations. Web ingestion depends on libcurl. The module is an initial native foundation, not yet equivalent to mature Python libraries such as NumPy, PyTorch, or scikit-learn. Separate modules now provide initial optimizer kernels, mini-batches, train/test splitting, basic losses, a dense forward pass, and evaluation metrics; general reverse-mode automatic differentiation, multi-layer training, GPU execution, richer dataset loaders, experiment tracking, and versioned model checkpoints remain future work.

**Design rule:** add future AI/ML capabilities as native module APIs or separate native modules. Keep greenServeFE core language and VM unchanged.

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


## Unified native AI/ML module

The eight AI/ML C implementations are consolidated into one native shared-library module, `gs_brain`. The greenServeFE language, compiler, VM, value representation, and `GSNativeAPI` ABI remain unchanged. All subsystem APIs are exposed as namespaces from the single module:

| Namespace | Main capabilities |
|---|---|
| `gs_brain.array` | Matrix operations, dot/matrix multiplication, reductions, statistics |
| `gs_brain.neural` | Linear and small dense ReLU regression, prediction, activations, losses |
| `gs_brain.optimizer` | SGD, momentum, Adam update kernels, gradient clipping |
| `gs_brain.dataset` | Seeded shuffling, mini-batches, one-hot encoding, train/test splits |
| `gs_brain.learn` | Logistic regression, standardization, regression/classification metrics |
| `gs_brain.autograd` | Scalar reverse-mode computation graph and backward propagation |
| `gs_brain.checkpoint` | Versioned numeric-vector save/load with checksum validation |
| `gs_brain.cortex` | Text/data ingestion, cleaning, tokenization, token counts, workspace helpers |

Example (after installing the compiled `gs_brain.so` where greenServeFE loads native modules):

```gsve
import gs_brain;

print(gs_brain.array.shape([[1, 2], [3, 4]]));
print(gs_brain.array.matmul([[1, 2], [3, 4]], [[5, 6], [7, 8]]));
print(gs_brain.cortex.workspace());
```

Run **Actions → Build gs_brain native AI module → Run workflow** to compile and package one Linux x86-64 shared library. The workflow is manually triggered and uploads `gs_brain.so`, source, ABI headers, a ZIP archive, and SHA-256 checksums. A successful compile/package job does not by itself establish runtime integration or numerical conformance. This is a CPU-first foundation, not full API parity with NumPy, PyTorch, or scikit-learn. See [the AI ecosystem plan](docs/AI-ECOSYSTEM.md) and [AI ecosystem news](docs/AI-NEWS.md).
