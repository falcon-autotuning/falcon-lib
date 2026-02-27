# ==============================================================================
# Tree-sitter grammar for Falcon DSL (.fal)
# ==============================================================================
# Prerequisites: Node.js >= 18, npm
# Install tree-sitter CLI:   npm install -g tree-sitter-cli
#                        or: npm install  (uses local devDependency)
# ==============================================================================

TREE_SITTER ?= tree-sitter
# If you prefer to use the local install from node_modules:
# TREE_SITTER = npx tree-sitter

.PHONY: all install generate test parse lint clean nvim-install help

# Default: install deps, generate parser, run tests
all: install generate test

## install     – Install Node dependencies (tree-sitter-cli etc.)
install:
	npm install

## generate    – Regenerate src/parser.c from grammar.js (requires Node)
##               Commit src/parser.c so consumers don't need Node.
generate: grammar.js
	$(TREE_SITTER) generate
	@echo "✔ parser.c generated in src/"

## test        – Run tree-sitter corpus tests in test/corpus/
test:
	$(TREE_SITTER) test
	@echo "✔ all corpus tests passed"

## test-filter – Run only tests matching FILTER=<pattern>
##               Usage: make test-filter FILTER="Autotuner"
test-filter:
	$(TREE_SITTER) test --filter "$(FILTER)"

## parse       – Parse a single .fal file for debugging
##               Usage: make parse FILE=path/to/file.fal
parse:
	$(TREE_SITTER) parse $(FILE)

## highlight   – Test highlights on a .fal file (terminal output)
##               Usage: make highlight FILE=path/to/file.fal
highlight:
	$(TREE_SITTER) highlight $(FILE)

## lint        – Check grammar.js for basic issues (generate in dry-run)
lint: grammar.js
	$(TREE_SITTER) generate --report-states-for-rule program
	@echo "✔ grammar lint OK"

## nvim-install – Register this grammar with nvim-treesitter (local dev)
##               Copies parser into Neovim's parser cache.
##               Requires Neovim with nvim-treesitter installed.
nvim-install: generate
	$(TREE_SITTER) build --wasm
	@echo ""
	@echo "For Neovim, add to your init.lua:"
	@echo "  See ../neovim/README.md for full instructions"

## clean       – Remove generated files (keeps grammar.js, queries/, tests/)
clean:
	rm -rf src/parser.c src/tree_sitter/ node_modules/ build/

## help        – Show this help message
help:
	@grep -E '^## ' Makefile | sed 's/## /  /'
