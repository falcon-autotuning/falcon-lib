# ==============================================================================
# Falcon LSP for Falcon DSL (.fal)
# ==============================================================================
# Prerequisites: cmake, ninja, clang, /opt/falcon installed
#
# Quick-start for local Neovim dev (LazyVim):
#   make nvim-install      – deploy Lua plugin to ~/.config/nvim/lua/plugins/
# ==============================================================================

# Neovim paths (override with: make nvim-install NVIM_LUA_DIR=...)
NVIM_LUA_DIR ?= $(HOME)/.config/nvim/lua/plugins

# Name of the Lua plugin file written into NVIM_LUA_DIR
NVIM_LSP_LUA = $(NVIM_LUA_DIR)/falcon-lsp.lua

# LSP binary installed by the build system
LSP_BIN ?= /opt/falcon/lib/falcon-lsp

.PHONY: nvim-install clean help

## nvim-install   – Deploy falcon-lsp LazyVim plugin to Neovim lua/plugins/
##                  Copies falcon_lsp.lua (plugin spec) and ftdetect/falcon.vim
##                  (filetype detection). Safe to run repeatedly.
nvim-install:
	@mkdir -p $(NVIM_LUA_DIR) $(NVIM_FTDETECT_DIR)
	cp neovim/falcon_lsp.lua $(NVIM_LUA_DIR)/falcon_lsp.lua
	cp neovim/ftdetect/falcon.vim $(NVIM_FTDETECT_DIR)/falcon.vim
	@echo ""
	@echo "✔ falcon_lsp.lua → $(NVIM_LUA_DIR)/falcon_lsp.lua"
	@echo "✔ falcon.vim     → $(NVIM_FTDETECT_DIR)/falcon.vim"
	@echo ""
	@echo "Restart Neovim and open a .fal file."
	@echo "The LSP binary is expected at: $(LSP_BIN)"
	@echo "Verify with :LspInfo inside a .fal buffer."

## clean          – Remove generated Lua files
clean:
	rm -f $(NVIM_LSP_LUA) $(NVIM_LUA_DIR)/falcon_lsp.lua
	@echo "✔ cleaned"

## help           – Show this help message
help:
	@grep -E '^## ' Makefile | sed 's/## /  /'
