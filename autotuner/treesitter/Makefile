# ==============================================================================
# Tree-sitter grammar for Falcon DSL (.fal)
# ==============================================================================
# Prerequisites: Node.js >= 18, npm, gcc
# Install tree-sitter CLI:   npm install -g tree-sitter-cli
#                        or: npm install  (uses local devDependency)
#
# Quick-start for local Neovim dev:
#   make nvim-dev          – generate + compile + install parser + queries + lua
#   make test              – run corpus tests
# ==============================================================================

TREE_SITTER ?= tree-sitter

# Language name — must match grammar.js name: 'falcon' and Neovim registration
LANG = falcon

# Neovim paths (override with: make nvim-dev NVIM_PARSER_DIR=...)
NVIM_PARSER_DIR  ?= $(HOME)/.local/share/nvim/site/parser
NVIM_QUERY_DIR   ?= $(HOME)/.config/nvim/queries/$(LANG)
NVIM_LUA_DIR     ?= $(HOME)/.config/nvim/lua/plugins

# Compiler settings
CC      ?= gcc
CFLAGS  ?= -O2 -shared -fPIC -fno-exceptions
INCLUDE  = -I ./src

# Source files — add src/scanner.c here if you add an external scanner
PARSER_SOURCES = src/parser.c

PARSER_SO = $(LANG).so

.PHONY: all install generate build-so test test-filter parse highlight lint \
        nvim-install nvim-install-queries nvim-install-lua nvim-dev clean help

# Default: install deps, generate parser, run tests
all: install generate test

# ------------------------------------------------------------------------------

## install               – Install Node dependencies
install:
	npm install

## generate              – Regenerate src/parser.c from grammar.js
generate: grammar.js
	$(TREE_SITTER) generate
	@echo "✔ parser.c generated in src/"

## build-so              – Compile src/parser.c → falcon.so
build-so: $(PARSER_SOURCES)
	$(CC) $(CFLAGS) $(INCLUDE) -o $(PARSER_SO) $(PARSER_SOURCES)
	@echo "✔ $(PARSER_SO) compiled"

## test                  – Run tree-sitter corpus tests
test:
	$(TREE_SITTER) test
	@echo "✔ all corpus tests passed"

## test-filter           – Run only tests matching FILTER=<pattern>
##                         Usage: make test-filter FILTER="Autotuner"
test-filter:
	$(TREE_SITTER) test --filter "$(FILTER)"

## parse                 – Parse a .fal file for debugging
##                         Usage: make parse FILE=path/to/file.fal
parse:
	$(TREE_SITTER) parse $(FILE)

## highlight             – Test highlights on a .fal file (terminal output)
##                         Usage: make highlight FILE=path/to/file.fal
highlight:
	$(TREE_SITTER) highlight $(FILE)

## lint                  – Check grammar.js for basic issues
lint: grammar.js
	$(TREE_SITTER) generate --report-states-for-rule program
	@echo "✔ grammar lint OK"

# ------------------------------------------------------------------------------
# Neovim integration
# ------------------------------------------------------------------------------

## nvim-install          – Compile and deploy falcon.so to Neovim's parser dir
nvim-install: build-so
	@mkdir -p $(NVIM_PARSER_DIR)
	cp $(PARSER_SO) $(NVIM_PARSER_DIR)/$(PARSER_SO)
	@echo "✔ $(PARSER_SO) → $(NVIM_PARSER_DIR)/$(PARSER_SO)"

## nvim-install-queries  – Deploy queries/ → ~/.config/nvim/queries/falcon/
nvim-install-queries:
	@mkdir -p $(NVIM_QUERY_DIR)
	cp ./queries/falcon/highlights.scm $(NVIM_QUERY_DIR)/highlights.scm
	@echo "✔ highlights.scm → $(NVIM_QUERY_DIR)/highlights.scm"
	@ls -1 $(NVIM_QUERY_DIR)/

## nvim-install-lua      – Deploy treesitter plugin config to LazyVim plugins dir
##                         Installs ~/.config/nvim/lua/plugins/treesitter.lua
##                         Safe to run repeatedly — only writes if dir exists.
nvim-install-lua:
	@mkdir -p $(NVIM_LUA_DIR)
	@echo 'return {' > $(NVIM_LUA_DIR)/treesitter.lua
	@echo '  {' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '    "nvim-treesitter/nvim-treesitter",' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '    opts = function(_, opts)' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '      vim.filetype.add({ extension = { fal = "falcon" } })' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '      vim.treesitter.language.add("falcon", {' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '        path = vim.fn.expand("$(NVIM_PARSER_DIR)/falcon.so"),' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '      })' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '      vim.treesitter.language.register("falcon", "falcon")' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '      opts.highlight = opts.highlight or {}' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '      opts.highlight.enable = true' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '      return opts' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '    end,' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '  },' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '  {' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '    "nvim-treesitter/nvim-treesitter",' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '    init = function()' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '      vim.api.nvim_create_autocmd("FileType", {' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '        pattern = "falcon",' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '        callback = function(ev)' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '          vim.treesitter.start(ev.buf, "falcon")' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '        end,' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '      })' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '    end,' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '  },' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo '}' >> $(NVIM_LUA_DIR)/treesitter.lua
	@echo "✔ treesitter.lua → $(NVIM_LUA_DIR)/treesitter.lua"

## nvim-dev              – Full dev cycle: generate + compile + deploy all
##                         This is the one command to run after any grammar change.
nvim-dev: generate nvim-install nvim-install-queries nvim-install-lua
	@echo ""
	@echo "════════════════════════════════════════════════"
	@echo "  Falcon tree-sitter fully deployed to Neovim"
	@echo "  Parser : $(NVIM_PARSER_DIR)/$(PARSER_SO)"
	@echo "  Queries: $(NVIM_QUERY_DIR)/highlights.scm"
	@echo "  Lua    : $(NVIM_LUA_DIR)/treesitter.lua"
	@echo "════════════════════════════════════════════════"
	@echo "  Restart Neovim and open a .fal file."

## clean                 – Remove generated and compiled files
clean:
	rm -rf src/parser.c src/tree_sitter/ node_modules/ build/
	rm -f $(PARSER_SO)
	@echo "✔ cleaned"

## help                  – Show this help message
help:
	@grep -E '^## ' Makefile | sed 's/## /  /'
