# Falcon LSP

A [Language Server Protocol](https://microsoft.github.io/language-server-protocol/) (LSP) implementation for the **Falcon autotuner DSL** (`.fal` files).

## Features

| Capability | Description |
|---|---|
| Diagnostics | Syntax errors and type-checking warnings published on every save |
| Hover | Show the type of a variable, parameter, state, or built-in function |
| Completion | Keywords, built-in functions, variables and state names in scope |
| Go-to Definition | Jump to the declaration of a variable, state, or autotuner |

## Installation

```bash
pip install .
# or for development
pip install -e ".[dev]"
```

## Running the server

```bash
falcon-lsp        # starts the LSP server over stdio
```

## Neovim setup

Copy `neovim/falcon_lsp.lua` to your Neovim config (e.g. `~/.config/nvim/lua/falcon_lsp.lua`) and add:

```lua
require('falcon_lsp').setup()
```

Make sure `nvim-lspconfig` is installed.

## Running tests

```bash
make test
# or directly
pytest tests/ -v
```

## Project structure

```
falcon_lsp/
├── lexer.py          # Tokeniser (mirrors lexer.l)
├── parser.py         # Recursive-descent parser (mirrors parser.y)
├── ast_nodes.py      # AST node dataclasses
├── type_checker.py   # Type inference and checking
├── builtins.py       # Built-in function registry
└── server.py         # pygls LSP server
tests/
├── test_lexer.py
├── test_parser.py
├── test_type_checker.py
├── test_server.py
└── integration/
    └── test_integration.py
neovim/
└── falcon_lsp.lua    # Neovim LSP configuration
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

`autotuner` `routine` `state` `start` `uses` `terminal` `if` `elif` `else` `true` `false` `nil` `config`

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
| `write` | `(scope: string, name: string, value: union<int\|float\|bool\|string>, ...) -> Error` |
