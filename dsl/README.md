# Falcon DSL

A domain-specific language and runtime for defining and executing **quantum device autotuning state machines**.

---

## Table of Contents

- [Overview](#overview)
- [Components](#components)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [The Falcon Language (.fal)](#the-falcon-language-fal)
- [The Interpreter / AutotunerEngine](#the-interpreter--autotunerengine)
- [CLI: `falcon-run`](#cli-falcon-run)
- [CLI: `falcon-test`](#cli-falcon-test)
- [Package Manager (`falcon-pm`)](#package-manager-falcon-pm)
- [Language Server (falcon-lsp)](#language-server-falcon-lsp)
- [Documentation Index](#documentation-index)
- [Testing](#testing)
- [License](#license)

---

## Overview

The Falcon DSL library (`falcon-dsl`) lets you define complex quantum device control workflows as declarative state machines in `.fal` files, then execute them at runtime with hardware measurement functions written in C++.

```fal
autotuner ChargeStability (float gate_voltage, int sweep_steps) -> (bool stable) {
    stable = false;
    start -> initialize;

    state initialize {
        bool ok = device_init();
        if (ok == true) { -> sweep; }
        else            { -> error; }
    }

    state sweep {
        float current = measure_current(gate_voltage, sweep_steps);
        stable = current > 0.0;
        if (stable == true) { -> done; }
        else                { -> error; }
    }

    state done  { terminal; }
    state error { terminal; }
}
```

---

## Components

| Component | Description |
|-----------|-------------|
| `falcon-dsl` | Core runtime library: interpreter, autotuner engine, type system |
| `falcon-atc` | Compiler sub-library: lexer, parser, AST (internal) |
| `falcon-pm` | Package manager: resolves `import` paths, handles manifests |
| `falcon-lsp` | Language server: IDE integration for `.fal` files |
| `falcon-run` | CLI tool: run any autotuner from the command line |
| `falcon-test` | CLI tool: fixture-aware test runner for `.fal` test suites |

---

## Installation

### Prerequisites

- `falcon-typing` installed (`cd ../typing && make install`)
- `falcon-database` and `falcon-comms` installed
- vcpkg and its dependencies resolved

### Install the full DSL suite

```bash
cd dsl
make install
```

This single command calls three sub-targets in order:

| Sub-target | What it installs |
|------------|-----------------|
| `make install-dsl` | `libfalcon-dsl.so`, headers, CMake config, `falcon-run`, `falcon-test` binaries |
| `make install-lsp` | `falcon-lsp` language server binary, `libfalcon-lsp.a` |
| `make install-pm` | `libfalcon-pm.a`, `falcon-pm` binary, headers |

You can also install components individually:

```bash
make install-dsl    # runtime library + CLIs only
make install-lsp    # language server only
make install-pm     # package manager only
```

Default install prefix is `/opt/falcon`. Override with:

```bash
make install INSTALL_PREFIX=/usr/local
```

---

## Quick Start

### 1. Write a `.fal` file

```fal
// hello.fal
autotuner Hello -> (bool greeted) {
    greeted = false;
    start -> greet;

    state greet {
        greeted = true;
        -> done;
    }

    state done { terminal; }
}
```

### 2. Run it with the CLI

```bash
falcon-run Hello hello.fal
```

Expected output:

```
Autotuner 'Hello' completed.
Results (1):
  [0] true
```

### 3. Embed in C++

```cpp
#include <falcon-dsl/AutotunerEngine.hpp>

int main() {
    falcon::dsl::AutotunerEngine engine;
    engine.load_fal_file("hello.fal");

    falcon::typing::ParameterMap inputs;
    auto results = engine.run_autotuner("Hello", inputs);
    // results[0] == true  (the 'greeted' output)
    return 0;
}
```

---

## The Falcon Language (`.fal`)

`.fal` files define one or more **autotuners** — finite state machines that call C++ measurement functions as transitions execute.

### File structure

```fal
// Optional imports
import "other_module.fal";

// Optional struct definitions — may be generic
struct SweepConfig {
    float start = 0.0;
    float stop  = 1.0;
}

struct Box <T> {
    T value;
    routine New (T v) -> (Box<T> b) { b.value = v; }
    routine Get      -> (T out)     { out = this.value; }
}

// One or more autotuner declarations
autotuner MyAutotuner (int iteration, float voltage) -> (float result) {
    result = 0.0;
    start -> init;

    state init { ... }
    state run  { ... }
    state done { terminal; }
}
```

### Language primitives

| Type | Description | Examples |
|------|-------------|---------|
| `int` | 64-bit signed integer | `0`, `-5`, `1000` |
| `float` | Double-precision float | `0.0`, `-3.14`, `1e-9` |
| `bool` | Boolean | `true`, `false` |
| `string` | String literal | `"hello"`, `"dev_0"` |

### Generic structs

Structs can be parameterised with type variables using angle-bracket syntax:

```fal
struct Accumulator <T> {
    T total;

    routine New (T init) -> (Accumulator<T> acc) {
        acc.total = init;
    }

    routine Add (T delta) -> (T new_total) {
        total     = total + delta;
        new_total = total;
    }
}

autotuner Example (int start, int delta) -> (int result) {
    result = 0;
    start -> run;
    state run {
        Accumulator<int> acc = Accumulator.New(start);
        result = acc.Add(delta);
        terminal;
    }
}
```

The interpreter resolves concrete types on demand — no manual template instantiation or separate compilation step is required.

### State anatomy

```fal
state sweep_point (float voltage) {
    // Received from the previous transition as a parameter

    // Scratch variables, local to this state
    float current = measure_iv(voltage);
    bool  valid   = current > 0.0;

    // Conditional transitions
    if (valid == true) { -> record(current, voltage); }
    else               { -> error; }
}
```

### Transitions

| Syntax | Meaning |
|--------|---------|
| `-> next_state;` | Unconditional jump |
| `if (cond) -> s1; else -> s2;` | Conditional branch |
| `-> next(val);` | Transfer value to state parameter |
| `-> Other::state;` | Cross-module jump |

### Routines

Pure `.fal` helper functions (no measurement, no state machine):

```fal
routine clamp (float val, float lo, float hi) -> (float out) {
    if (val < lo) { out = lo; }
    elif (val > hi) { out = hi; }
    else { out = val; }
}
```

Cross-module routines are called by qualified name — no `uses` declaration is needed:

```fal
float safe = math_utils::clamp(input, 0.0, 1.0);
```

### Imports

```fal
// Single path
import "shared/types.fal";

// Multi-path
import (
    "shared/types.fal"
    "shared/routines.fal"
)
```

### FFI (Foreign Function Interface)

Bind a C++ shared library wrapper to `.fal` symbols. The engine compiles it automatically at load time via the C ABI — no separate build step is needed:

```fal
ffimport "hardware_wrapper.cpp"
    ("-I/opt/mydevice/include")
    ("-lmydevice")
```

---

## The Interpreter / AutotunerEngine

`AutotunerEngine` is the central C++ API for loading and running autotuners.

```cpp
#include <falcon-dsl/AutotunerEngine.hpp>

falcon::dsl::AutotunerEngine engine;

// Load one or more .fal files (imports and ffimport wrappers resolved automatically)
engine.load_fal_file("sweep.fal");

// Build inputs
falcon::typing::ParameterMap inputs;
inputs["min_voltage"] = 0.0;
inputs["max_voltage"] = 1.0;

// Run
auto results = engine.run_autotuner("VoltageSweep", inputs);

// Introspect loaded content
for (auto &name : engine.get_loaded_autotuners())
    std::cout << "Autotuner: " << name << "\n";
```

### Key methods

| Method | Description |
|--------|-------------|
| `load_fal_file(path)` | Parse and load a `.fal` file; recursively resolves imports and compiles `ffimport` wrappers |
| `run_autotuner(name, inputs)` | Execute a named autotuner, returns `FunctionResult` |
| `run_routine(name, inputs)` | Execute a standalone routine |
| `get_loaded_autotuners()` | List all loaded autotuner names |
| `has_autotuner(name)` | Check whether an autotuner is loaded |

---

## CLI: `falcon-run`

`falcon-run` lets you execute any autotuner from the terminal without writing C++ glue code.

### Usage

```bash
falcon-run <autotuner-name> <file.fal> [more.fal ...] [options]
```

### Options

| Option | Description |
|--------|-------------|
| `--list` | List all autotuners found in the loaded files |
| `--param k=v` | Pass an initial parameter (repeatable); type is inferred |
| `--log-level L` | Set log level: `trace`, `debug`, `info`, `warn`, `error` |
| `--help` | Print help |

### Exit codes

| Code | Meaning |
|------|---------|
| `0` | Success |
| `1` | Usage / argument error |
| `2` | File load failure |
| `3` | Runtime error |

### Examples

```bash
# Run an autotuner
falcon-run ChargeStability charge.fal

# Run with parameters
falcon-run VoltageSweep sweep.fal --param min_voltage=0.0 --param max_voltage=1.5

# List autotuners in a file
falcon-run --list my_autotuners.fal

# Debug output
falcon-run MyAutotuner file.fal --log-level debug
```

If your autotuner uses `ffimport`, the engine compiles the C++ wrapper automatically — no extra build step is required.

See [docs/CLI.md](docs/CLI.md) for the full CLI reference.

---

## CLI: `falcon-test`

`falcon-test` is the dedicated test runner for Falcon DSL test suites. It provides gtest-style coloured output, per-test `setup`/`teardown` fixture support, and non-aborting assertions so all failures within a test are always reported.

### How it works

`falcon-test` reads one or more `.fal` test files. It discovers **test suite autotuners** — autotuners with the signature `-> (int passed, int failed)` — and for each one it:

1. Scans the body for `state test_*` declarations to build the test list.
2. Detects optional `state setup` and `state teardown` declarations.
3. Generates and injects harness states (`__init`, `__loop`, `__begin`, `__dispatch`, `__end`, `__finish`) that manage the test loop and output.
4. Writes the expanded source next to the original file so all `import` paths continue to resolve correctly.
5. Loads and runs each suite through `AutotunerEngine`.

Users never write or see the harness states. Their complete contract is:

```fal
import "/opt/falcon/libs/testing/testing.fal";

autotuner MySuite -> (int passed, int failed) {
    passed = 0; failed = 0;
    start -> __init;           // only required boilerplate line

    // optional — runs before every test
    state setup (TestRunner runner) {
        // reset environment
        -> __begin(runner);    // required terminal transition for setup
    }

    // optional — runs after every test
    state teardown (TestRunner runner, TestContext t) {
        // cleanup
        -> __end(runner, t);   // required terminal transition for teardown
    }

    // one state per test — name must start with test_
    state test_my_feature (TestRunner runner, TestContext t) {
        Error err = t.ExpectIntEq(1 + 1, 2, "addition works");
        -> teardown(runner, t);    // or -> __end(runner, t) if no teardown
    }
}
```

All `import` statements, structs (including generic ones), routines, and other autotuners defined in the same file are available to test bodies as normal.

### Usage

```bash
falcon-test <file.fal> [file2.fal ...] [options]
```

### Options

| Option | Description |
|--------|-------------|
| `--log-level L` | Set log level: `trace`, `debug`, `info`, `warn`, `error` (default: `warn`) |
| `--dump` | Print the generated `.fal` source and exit — useful for debugging |
| `--help` | Print help |

### Exit codes

| Code | Meaning |
|------|---------|
| `0` | All tests in all suites passed |
| `1` | Usage / argument error |
| `2` | File load or generation error |
| `3` | One or more tests failed |

### Examples

```bash
# Run a single test file
falcon-test my_tests.fal

# Run multiple files — all suites run, combined exit code
falcon-test unit_tests.fal integration_tests.fal

# Inspect the generated harness source without running
falcon-test my_tests.fal --dump

# Suppress engine log noise in CI
falcon-test my_tests.fal --log-level error
```

### Terminal output

```
falcon-test: my_tests.fal
  suite [MySuite]  3 test(s)  setup  teardown

[==========] Running 3 test(s) from MySuite
[----------] Global test environment set-up.
[ RUN      ] MySuite.test_addition
[       OK ] MySuite.test_addition
[ RUN      ] MySuite.test_bad_math
             ✗ 1 != 2 — mismatch (expected=1 actual=2)
[  FAILED  ] MySuite.test_bad_math
[ RUN      ] MySuite.test_strings
[       OK ] MySuite.test_strings

[==========] 3 test(s) from MySuite
[  PASSED  ] 2 test(s)
[  FAILED  ] 1 test(s)
[==========] 3 test(s) ran.
```

See [libs/testing/README.md](../libs/testing/README.md) for the full testing library reference.

---

## Package Manager (`falcon-pm`)

The package manager resolves `import` statements in `.fal` files, handling both relative paths and manifest-declared packages.

### `falcon.yml` manifest

Place a `falcon.yml` next to your entry `.fal` file:

```yaml
name: my-autotuner-suite
version: 1.0.0
dependencies:
  shared-routines: "../shared"
```

### Import resolution order

1. Relative to the importing file
2. Relative to the project root (where `falcon.yml` lives)
3. Paths declared in `dependencies` section of `falcon.yml`
4. System package paths (`/opt/falcon/packages/`)

### CLI (`falcon-pm`)

```bash
# Show resolved imports for a file
falcon-pm resolve my_autotuner.fal

# Install packages declared in falcon.yml
falcon-pm install
```

See [docs/PACKAGE_MANAGER.md](docs/PACKAGE_MANAGER.md) for the full guide.

---

## Language Server (`falcon-lsp`)

`falcon-lsp` provides IDE integration for `.fal` files via the Language Server Protocol.

### Features

- Syntax error diagnostics
- Completion for state names, parameters, keywords, and generic type parameters
- Go-to-definition for states, routines, and struct types
- Hover documentation
- Rename refactoring

### Installation

```bash
make install-lsp
# Installs falcon-lsp to /opt/falcon/lib/falcon-lsp
```

### Editor setup

**VS Code** — install the `falcon-dsl` VS Code extension (searches the marketplace for `falcon-lsp`).

**Neovim (nvim-lspconfig)**:

```lua
local lspconfig = require('lspconfig')
lspconfig.falcon_lsp.setup {
  cmd = { '/opt/falcon/lib/falcon-lsp' },
  filetypes = { 'fal' },
  root_dir = lspconfig.util.root_pattern('falcon.yml', '.git'),
}
```

A ready-made lazy.nvim snippet is at `dsl/lsp/neovim/init.lua`.

See [docs/LSP.md](docs/LSP.md) for setup guides for all major editors.

---

## Documentation Index

| Document | Description |
|----------|-------------|
| [docs/LANGUAGE_REFERENCE.md](docs/LANGUAGE_REFERENCE.md) | Complete `.fal` syntax and language reference |
| [docs/TUTORIAL.md](docs/TUTORIAL.md) | Step-by-step tutorial: build your first autotuner |
| [docs/CLI.md](docs/CLI.md) | `falcon-run` and `falcon-test` CLI reference |
| [docs/LSP.md](docs/LSP.md) | Language server setup for all editors |
| [docs/PACKAGE_MANAGER.md](docs/PACKAGE_MANAGER.md) | Package manager guide |
| [libs/testing/README.md](../libs/testing/README.md) | Testing library: `TestContext`, `TestRunner`, fixture patterns |

---

## Testing

```bash
# All tests (requires Docker for PostgreSQL + NATS)
make test

# Without Docker (services must be running externally)
export TEST_DATABASE_URL=postgresql://falcon_test:falcon_test_password@127.0.0.1:5433/falcon_test
export TEST_NATS_URL=nats://localhost:4222
make test-local

# Run the Falcon DSL self-tests with falcon-test
falcon-test libs/testing/tests/run_tests.fal
falcon-test libs/testing/tests/error_detection.fal || true  # failures expected
```

---

## License

MPL-2.0
