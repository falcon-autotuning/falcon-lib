# Falcon LSP — Language Server Setup

`falcon-lsp` implements the Language Server Protocol for `.fal` files, providing IDE features such as diagnostics, completions, hover documentation, and go-to-definition.

---

## Features

| Feature | Description |
|---------|-------------|
| Diagnostics | Syntax and semantic errors highlighted in real time |
| Completion | Keywords, state names, parameter names, routine names |
| Go-to-definition | Jump to a state, routine, or struct declaration |
| Hover | Show type information and doc comments |
| Rename | Rename a state or variable across the file |
| Document symbols | Outline of autotuners, states, and routines |

---

## Installation

```bash
cd dsl
make install-lsp
```

This installs:

- `/opt/falcon/lib/falcon-lsp` — the LSP server binary
- `/opt/falcon/lib/libfalcon-lsp.a` — static library for embedding

---

## Communication

`falcon-lsp` communicates over **stdin/stdout** using the standard LSP JSON-RPC wire protocol. Most editors that support LSP will handle this automatically.

---

## Editor setup

### Neovim (nvim-lspconfig)

A ready-made configuration snippet is provided at `dsl/lsp/neovim/init.lua`. For quick setup:

```lua
-- ~/.config/nvim/init.lua (or your preferred config location)
local lspconfig = require('lspconfig')

lspconfig.falcon_lsp = {
  default_config = {
    cmd        = { '/opt/falcon/lib/falcon-lsp' },
    filetypes  = { 'fal' },
    root_dir   = lspconfig.util.root_pattern('falcon.yml', '.git'),
    settings   = {},
  },
}

lspconfig.falcon_lsp.setup {
  on_attach = function(client, bufnr)
    local opts = { buffer = bufnr }
    vim.keymap.set('n', 'K',           vim.lsp.buf.hover,        opts)
    vim.keymap.set('n', 'gd',          vim.lsp.buf.definition,   opts)
    vim.keymap.set('n', '<C-Space>',   vim.lsp.buf.completion,   opts)
    vim.keymap.set('n', '<leader>rn',  vim.lsp.buf.rename,       opts)
    vim.keymap.set('n', '<leader>e',   vim.diagnostic.open_float, opts)
  end,
}

-- Register .fal filetype
vim.filetype.add({ extension = { fal = 'fal' } })
```

For **lazy.nvim** users, use `dsl/lsp/neovim/init.lua` directly as a plugin spec:

```lua
-- lazy.nvim spec
{ import = "path/to/dsl/lsp/neovim/init" }
```

### VS Code

1. Install the **Falcon DSL** extension from the VS Code marketplace.
2. The extension automatically locates `falcon-lsp` if it is on your `PATH` or at `/opt/falcon/lib/falcon-lsp`.

To set a custom binary path, add to your `settings.json`:

```json
{
  "falconDsl.languageServerPath": "/opt/falcon/lib/falcon-lsp"
}
```

### Emacs (eglot)

```elisp
(add-to-list 'eglot-server-programs
             '(falcon-mode . ("/opt/falcon/lib/falcon-lsp")))

(add-hook 'falcon-mode-hook 'eglot-ensure)
```

### Helix

Add to `~/.config/helix/languages.toml`:

```toml
[[language]]
name = "fal"
scope = "source.fal"
file-types = ["fal"]
roots = ["falcon.yml"]
language-servers = ["falcon-lsp"]

[language-server.falcon-lsp]
command = "/opt/falcon/lib/falcon-lsp"
```

---

## TreeSitter syntax highlighting

A TreeSitter grammar is included at `dsl/treesitter/`. The compiled parser is at `dsl/treesitter/src/parser.c`.

### Neovim (nvim-treesitter)

```lua
local parser_config = require("nvim-treesitter.parsers").get_parser_configs()
parser_config.falcon = {
  install_info = {
    url   = "/path/to/falcon-lib/dsl/treesitter",
    files = { "src/parser.c" },
    branch = "main",
  },
  filetype = "fal",
}
```

Then run `:TSInstall falcon`.

---

## Diagnostics reference

| Error code | Cause |
|------------|-------|
| `E001` | Undefined state referenced in transition |
| `E002` | Undefined variable in expression |
| `E003` | Missing `terminal` on terminal state |
| `E004` | Type mismatch in assignment |
| `E005` | Undeclared measurement function |
| `W001` | Unreachable state |
| `W002` | Unused persistent parameter |
