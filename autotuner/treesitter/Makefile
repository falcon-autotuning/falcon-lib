# ==============================================================================
# Tree-sitter grammar for Falcon DSL (.fal)
# ==============================================================================
# Prerequisites: Node.js >= 18, npm, gcc
# Install tree-sitter CLI:   npm install -g tree-sitter-cli
#                        or: npm install  (uses local devDependency)
#
# Quick-start for local Neovim dev:
#   make nvim-dev          – generate + compile + install parser + queries
#   make test              – run corpus tests
#   make nvim-install      – compile and deploy .so only (parser already generated)
# ==============================================================================

TREE_SITTER ?= tree-sitter
# If you prefer to use the local install from node_modules:
# TREE_SITTER = npx tree-sitter

# Language name — must match grammar.js name: '...' and Neovim registration
LANG = falcon

# Where Neovim looks for parser .so files (override with: make nvim-install NVIM_PARSER_DIR=...)
NVIM_PARSER_DIR ?= $(HOME)/.local/share/nvim/site/parser

# Where Neovim looks for query files
NVIM_QUERY_DIR ?= $(HOME)/.config/nvim/queries/$(LANG)

# Compiler settings
CC      ?= gcc
CFLAGS  ?= -O2 -shared -fPIC -fno-exceptions
INCLUDE  = -I ./src

# Source files — add src/scanner.c here if you add an external scanner
PARSER_SOURCES = src/parser.c

# Output .so name
PARSER_SO = $(LANG).so

.PHONY: all install generate build-so test test-filter parse highlight lint \
        nvim-install nvim-install-queries nvim-dev clean help

# Default: install deps, generate parser, run tests
all: install generate test

# ------------------------------------------------------------------------------

## install            – Install Node dependencies (tree-sitter-cli etc.)
install:
	npm install

## generate           – Regenerate src/parser.c from grammar.js (requires Node)
##                      Commit src/parser.c so consumers don't need Node.
generate: grammar.js
	$(TREE_SITTER) generate
	@echo "✔ parser.c generated in src/"

## build-so           – Compile src/parser.c → falcon.so (requires gcc)
##                      Does NOT regenerate parser.c — run generate first if grammar changed.
build-so: $(PARSER_SOURCES)
	$(CC) $(CFLAGS) $(INCLUDE) -o $(PARSER_SO) $(PARSER_SOURCES)
	@echo "✔ $(PARSER_SO) compiled"

## test               – Run tree-sitter corpus tests in test/corpus/
test:
	$(TREE_SITTER) test
	@echo "✔ all corpus tests passed"

## test-filter        – Run only tests matching FILTER=<pattern>
##                      Usage: make test-filter FILTER="Autotuner"
test-filter:
	$(TREE_SITTER) test --filter "$(FILTER)"

## parse              – Parse a single .fal file for debugging
##                      Usage: make parse FILE=path/to/file.fal
parse:
	$(TREE_SITTER) parse $(FILE)

## highlight          – Test highlights on a .fal file (terminal output)
##                      Usage: make highlight FILE=path/to/file.fal
highlight:
	$(TREE_SITTER) highlight $(FILE)

## lint               – Check grammar.js for basic issues
lint: grammar.js
	$(TREE_SITTER) generate --report-states-for-rule program
	@echo "✔ grammar lint OK"

# ------------------------------------------------------------------------------
# Neovim integration
# ------------------------------------------------------------------------------

## nvim-install       – Compile parser and deploy .so to Neovim's parser directory.
##                      Run this after any grammar.js change:
##                        make generate nvim-install
##                      Or use the combined shortcut:
##                        make nvim-dev
nvim-install: build-so
	@mkdir -p $(NVIM_PARSER_DIR)
	cp $(PARSER_SO) $(NVIM_PARSER_DIR)/$(PARSER_SO)
	@echo "✔ $(PARSER_SO) → $(NVIM_PARSER_DIR)/$(PARSER_SO)"
	@echo ""
	@echo "Restart Neovim or run :lua vim.cmd('edit') to reload the parser."

## nvim-install-queries – Sync queries/ → ~/.config/nvim/queries/falcon/
##                        Run this after any .scm file change.
nvim-install-queries:
	@mkdir -p $(NVIM_QUERY_DIR)
	cp -r queries/falcon/. $(NVIM_QUERY_DIR)/
	@echo "✔ queries/ → $(NVIM_QUERY_DIR)/"
	@echo ""
	@echo "Query files deployed:"
	@ls -1 $(NVIM_QUERY_DIR)/

## nvim-dev            – Full dev cycle: generate → compile → deploy parser + queries.
##                      Use this as your one-stop command during grammar development.
nvim-dev: generate nvim-install nvim-install-queries
	@echo ""
	@echo "════════════════════════════════════════"
	@echo "  Falcon tree-sitter deployed to Neovim"
	@echo "  Parser : $(NVIM_PARSER_DIR)/$(PARSER_SO)"
	@echo "  Queries: $(NVIM_QUERY_DIR)/"
	@echo "════════════════════════════════════════"
	@echo "  Restart Neovim and open a .fal file."

## clean              – Remove generated and compiled files
##                      (keeps grammar.js, queries/, test/)
clean:
	rm -rf src/parser.c src/tree_sitter/ node_modules/ build/
	rm -f $(PARSER_SO)
	@echo "✔ cleaned"

## help               – Show this help message
help:
	@grep -E '^## ' Makefile | sed 's/## /  /'
