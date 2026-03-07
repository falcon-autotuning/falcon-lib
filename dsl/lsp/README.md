# Falcon LSP

A [Language Server Protocol](https://microsoft.github.io/language-server-protocol/) (LSP) server implemented in **C++20** for the **Falcon autotuner DSL** (`.fal` files).

## Features

| Capability | Description |
|---|---|
| Diagnostics | Syntax errors published on every open/change/save (red squiggles) |
| Hover | Show the type of a variable, parameter, state, or built-in function (`K`) |
| Completion | Keywords, built-in functions, variables and state names in scope |
| Go-to Definition | Jump to the declaration of a variable or state |

## Running the server

```bash
falcon-lsp        # LSP server over stdio (used by editors)
falcon-lint *.fal # Standalone linter, exits 0 on success
```

## Neovim / LazyVim setup

### Quick install (recommended)

```bash
cd dsl/lsp
make nvim-install
```

This installs two files:

| File | Destination |
|---|---|
| `neovim/falcon_lsp.lua` | `~/.config/nvim/lua/plugins/falcon_lsp.lua` |
| `neovim/ftdetect/falcon.vim` | `~/.config/nvim/ftdetect/falcon.vim` |

Restart Neovim, open any `.fal` file, and run `:LspInfo` to verify the server is attached.

### Keymaps (set automatically on attach)

| Key | Action |
|---|---|
| `K` | Hover — show type / docs |
| `gd` | Go to definition |
| `<C-Space>` | Trigger completion |
| `<leader>rn` | Rename symbol |
| `<leader>e` | Open diagnostics float |

Red squiggles (error diagnostics) appear automatically on every file open/change/save — no extra configuration needed.

### Manual install

```bash
cp dsl/lsp/neovim/falcon_lsp.lua     ~/.config/nvim/lua/plugins/falcon_lsp.lua
cp dsl/lsp/neovim/ftdetect/falcon.vim ~/.config/nvim/ftdetect/falcon.vim
```

Requires `neovim/nvim-lspconfig` (already included in LazyVim).

## TreeSitter

A TreeSitter grammar for syntax highlighting is in `dsl/treesitter/grammar.js`.
See `dsl/treesitter/README.md` for installation instructions.

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
| `write` | `(scope: string, name: string, value: ...) -> Error` |
