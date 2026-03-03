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
- [Package Manager (`falcon-pm`)](#package-manager-falcon-pm)
- [Language Server (falcon-lsp)](#language-server-falcon-lsp)
- [Documentation Index](#documentation-index)
- [Testing](#testing)
- [License](#license)

---

## Overview

The Falcon DSL library (`falcon-dsl`) lets you define complex quantum device control workflows as declarative state machines in `.fal` files, then execute them at runtime with hardware measurement functions written in C++.

```fal
autotuner ChargeStability {
  params {
    float gate_voltage = 0.0;
    int   sweep_steps  = 50;
  }

  start -> initialize;

  state initialize {
    temp { bool ok; }
    measurement: device_init();
    if (ok == true) -> sweep;
    else            -> error;
  }

  state sweep {
    temp { float current; bool stable; }
    measurement: measure_current(gate_voltage, sweep_steps);
    if (stable == true) -> done;
    else                -> error;
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
| `make install-dsl` | `libfalcon-dsl.so`, headers, CMake config, `falcon-run` binary |
| `make install-lsp` | `falcon-lsp` language server binary, `libfalcon-lsp.a` |
| `make install-pm` | `libfalcon-pm.a`, `falcon-pm` binary, headers |

You can also install components individually:

```bash
make install-dsl    # runtime library + CLI only
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
autotuner Hello {
  params {
    bool greeted = false;
  }

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
    // results[0] == true (the 'greeted' param)
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

// Optional struct definitions
struct DeviceConfig {
  float bias     = 0.0;
  float gain     = 1.0;
}

// One or more autotuner declarations
autotuner MyAutotuner {
  uses  [OtherAutotuner];          // optional: cross-autotuner dependencies
  params {
    int   iteration = 0;
    float voltage   = 0.0;
  }

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

### State anatomy

```fal
state sweep_point {
  // Received from the previous transition
  params {
    float voltage;
  }

  // Scratch variables, live only in this state
  temp {
    float current;
    bool  valid;
  }

  // Calls a C++ function; output keys populate temp/params
  measurement: measure_iv(voltage);

  // Conditional transitions
  if (valid == true && current > 0.0) -> record[current, voltage];
  else                                -> error;
}
```

### Transitions

| Syntax | Meaning |
|--------|---------|
| `-> next_state;` | Unconditional jump |
| `if (cond) -> s1; else -> s2;` | Conditional branch |
| `-> next[var];` | Transfer `var` by name |
| `-> next[a: b];` | Transfer `a`, bind as `b` in target |
| `-> Other::state;` | Cross-autotuner jump |

### Routines

Pure `.fal` helper functions (no measurement, no state machine):

```fal
routine clamp (float val, float lo, float hi) -> (float out) {
  if (val < lo) { out = lo; }
  elif (val > hi) { out = hi; }
  else { out = val; }
}
```

### Structs

Composite types usable as parameter types:

```fal
struct SweepConfig {
  float start  = 0.0;
  float stop   = 1.0;
  int   points = 100;
}
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

Bind a C++ shared library wrapper to `.fal` symbols:

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

// Load one or more .fal files (imports resolved automatically)
engine.load_fal_file("sweep.fal");

// Optionally bind external C++ routines
engine.load_routine_library({
    .name         = "measure_iv",
    .library_path = "libhardware.so",
    .name_space   = "VoltageSweep",
});

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
| `load_fal_file(path)` | Parse and load a `.fal` file; recursively resolves imports |
| `load_routine_library(config)` | Bind a `.so` containing C++ measurement implementations |
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

### Inline measurement functions

If your autotuner's measurement functions are implemented as `.fal` routines (no C++), `falcon-run` works immediately. For autotuners that call external C++ measurement libraries, use `ffimport` in the `.fal` file so the engine can compile and link them automatically.

See [docs/CLI.md](docs/CLI.md) for the full CLI reference.

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
  device-types:    "/opt/falcon/packages/device-types"
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
- Completion for state names, parameters, and keywords
- Go-to-definition for states and routines
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
-- In your init.lua
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
| [docs/CLI.md](docs/CLI.md) | `falcon-run` CLI reference |
| [docs/LSP.md](docs/LSP.md) | Language server setup for all editors |
| [docs/PACKAGE_MANAGER.md](docs/PACKAGE_MANAGER.md) | Package manager guide |

---

## Testing

```bash
# All tests (requires Docker for PostgreSQL + NATS)
make test

# Without Docker (services must be running externally)
export TEST_DATABASE_URL=postgresql://falcon_test:falcon_test_password@127.0.0.1:5433/falcon_test
export TEST_NATS_URL=nats://localhost:4222
make test-local
```

---

## License

MPL-2.0
