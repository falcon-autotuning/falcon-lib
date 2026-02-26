# TreeSitter Grammar for Falcon DSL

This directory contains a [TreeSitter](https://tree-sitter.github.io/tree-sitter/) grammar
for `.fal` (Falcon Autotuner DSL) files.

## Installation

```bash
# From this directory
npm init -y
npm install tree-sitter-cli
npx tree-sitter generate
npx tree-sitter test
```

## Usage in Neovim (lazy.nvim)

Add `nvim-treesitter` to your plugin list and point it at this grammar:

```lua
require('nvim-treesitter.configs').setup({
  ensure_installed = { 'fal' },
})
```

See the `../neovim/` directory for full configuration examples.
