# Tree-sitter Grammar for Falcon DSL

This directory contains the [Tree-sitter](https://tree-sitter.github.io/tree-sitter/) grammar
and highlight queries for `.fal` (Falcon Autotuner DSL) files.

It lives at `autotuner/treesitter/` — parallel to the LSP server — because
tree-sitter is an independent tool used by editors for syntax highlighting and
structural queries, independent of the LSP.

## Directory structure

```
treesitter/
├── grammar.js              # Source of truth — edit this to change the grammar
├── tree-sitter.json        # tree-sitter CLI project config (name, file types)
├── package.json            # Node project (tree-sitter-cli devDependency)
├── src/
│   └── parser.c            # Generated — committed so consumers skip Node
├── queries/
│   └── highlights.scm      # Highlight capture names for Neovim / LazyVim
├── test/
│   └── corpus/
│       └── highlights.txt  # Corpus tests (input → expected parse tree)
├── Makefile                # All CLI commands documented
└── README.md
```

## Prerequisites

- **Node.js** ≥ 18 (only needed to regenerate `parser.c` from `grammar.js`)
- **tree-sitter CLI** — installed as a devDependency via `npm install`

## Quick start

```bash
cd autotuner/treesitter

# 1. Install CLI locally
npm install

# 2. Regenerate parser.c from grammar.js (re-run after grammar changes)
make generate

# 3. Run all corpus tests
make test

# 4. Parse a .fal file to inspect the AST
make parse FILE=../../examples/calculator.fal

# 5. Preview highlighting in terminal
make highlight FILE=../../examples/calculator.fal
```

## Should I commit `src/parser.c`?

**Yes.** This is the standard practice for all official tree-sitter grammars
(e.g. tree-sitter-javascript, tree-sitter-python). Committing the generated C
file means:

- Editors and plugin managers (nvim-treesitter) can compile the parser **without
  needing Node.js or the tree-sitter CLI**
- CI environments don't need the full Node toolchain to build
- Other contributors can use the parser immediately after `git clone`

Only re-generate it when `grammar.js` changes, then commit the updated `parser.c`.

## Adding/modifying the grammar

1. Edit `grammar.js`
2. Run `make generate` → updates `src/parser.c`
3. Run `make test` → verify corpus tests still pass
4. Add new corpus test cases to `test/corpus/highlights.txt`
5. Commit both `grammar.js` and `src/parser.c`

## Neovim / LazyVim setup

See [`../lsp/neovim/`](../lsp/neovim/) for the full LazyVim config. For
tree-sitter highlighting specifically:

```lua
-- In your LazyVim config (e.g. lua/plugins/treesitter.lua)
{
  "nvim-treesitter/nvim-treesitter",
  opts = function(_, opts)
    -- Register the local Falcon parser
    local parser_config = require("nvim-treesitter.parsers").get_parser_configs()
    parser_config.fal = {
      install_info = {
        -- Point at this directory (absolute path or relative to Neovim config)
        url = vim.fn.stdpath("config") .. "/../../autotuner/treesitter",
        files = { "src/parser.c" },
        branch = "main",
        generate_requires_npm = false,
        requires_generate_from_grammar = false,
      },
      filetype = "fal",
    }

    -- Associate .fal files with the parser
    vim.filetype.add({ extension = { fal = "fal" } })

    opts.ensure_installed = opts.ensure_installed or {}
    table.insert(opts.ensure_installed, "fal")
  end,
}
```

Then copy (or symlink) `queries/highlights.scm` into Neovim's runtime:

```bash
mkdir -p ~/.config/nvim/queries/fal
cp queries/highlights.scm ~/.config/nvim/queries/fal/highlights.scm
```

Then in Neovim: `:TSInstall fal` followed by `:TSUpdate fal`.

Verify highlighting is active on a `.fal` file with:

```
:TSBufInfo
:Inspect        (Neovim 0.9+, shows active highlight groups under cursor)
```
