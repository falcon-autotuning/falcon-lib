# Falcon LSP

A [Language Server Protocol](https://microsoft.github.io/language-server-protocol/) (LSP) server implemented in **C++20** for the **Falcon autotuner DSL** (`.fal` files).

## Features

| Capability | Description |
|---|---|
| Diagnostics | Syntax errors published on every open/change/save |
| Hover | Show the type of a variable, parameter, state, or built-in function |
| Completion | Keywords, built-in functions, variables and state names in scope |
| Go-to Definition | Jump to the declaration of a variable or state |

## Building

```bash
cd falcon-lsp
make build-debug    # Debug build with tests
make build-release  # Optimised release build
make test           # Build debug + run tests
make install        # Install to /opt/falcon (needs sudo)
```

Or manually with CMake:

```bash
mkdir -p build/debug && cd build/debug
cmake ../.. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTS=ON \
    -Dlsp_DIR=/opt/falcon/lib/cmake/lsp \
    -DGTest_DIR=/usr/local/lib/cmake/GTest \
    -G Ninja
ninja
ctest --output-on-failure
```

## Running the server

```bash
falcon-lsp        # LSP server over stdio (used by editors)
falcon-lint *.fal # Standalone linter, exits 0 on success
```

## Neovim setup

Copy `neovim/falcon_lsp.lua` to `~/.config/nvim/lua/falcon_lsp.lua` and add to your config:

```lua
require('falcon_lsp').setup()
```

See `neovim/init.lua` for a complete lazy.nvim example. Requires `nvim-lspconfig`.

## TreeSitter

A TreeSitter grammar for syntax highlighting is in `treesitter/grammar.js`.
See `treesitter/README.md` for installation instructions.

## Project structure

```
falcon-lsp/
├── CMakeLists.txt
├── Makefile
├── src/
│   ├── FalconDocument.{hpp,cpp}      # Document cache + parsing via falcon-atc-core
│   ├── TypeChecker.{hpp,cpp}         # AST walk → symbol table
│   ├── HoverProvider.{hpp,cpp}       # Hover logic
│   ├── CompletionProvider.{hpp,cpp}  # Completion logic
│   ├── DiagnosticsProvider.{hpp,cpp} # Parse errors → LSP diagnostics
│   ├── DefinitionProvider.{hpp,cpp}  # Go-to definition logic
│   ├── FalconServer.{hpp,cpp}        # Main LSP server class
│   └── main.cpp                      # Entry point
├── tools/falcon-lint/                # Standalone CLI linter
├── tests/
│   ├── unit/                         # Unit tests for each provider
│   └── integration/                  # Tests against real .fal files
├── neovim/                           # Neovim/lazy.nvim config
└── treesitter/                       # TreeSitter grammar
```

## Language overview

The Falcon DSL describes quantum-device autotuners as state machines:

```fal
autotuner Calculator (int a, int b) -> (int sum, int product) {
  sum = 0;
  product = 0;
  start -> calculate;

  state calculate {
    sum = a + b;
    product = a * b;
    -> done;
  }

  state done {
    terminal;
  }
}
```

### Keywords

`autotuner` `routine` `state` `start` `uses` `terminal` `if` `else` `true` `false` `nil`

### Types

`int` `float` `bool` `string` `Quantity` `Config` `Connection` `Connections` `Gname` `Error`

### Built-in functions

| Function | Signature |
|---|---|
| `logInfo` | `(format: string) -> void` |
| `logWarn` | `(format: string) -> void` |
| `logError` | `(format: string) -> void` |
| `errorMsg` | `(message: string) -> Error` |
| `fatalErrorMsg` | `(message: string) -> Error` |
| `readLatest` | `(scope: string, name: string, ...) -> (union<int\|float\|bool\|string>, Error)` |
| `write` | `(scope: string, name: string, value: ...) -> Error` |
