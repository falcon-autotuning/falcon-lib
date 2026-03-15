-- Example LazyVim configuration for falcon-lsp
--
-- Quick install (recommended):
--   cd dsl/lsp && make nvim-install
--
-- Manual install:
--   cp neovim/falcon_lsp.lua     ~/.config/nvim/lua/plugins/falcon_lsp.lua
--   cp neovim/ftdetect/falcon.vim ~/.config/nvim/ftdetect/falcon.vim
--
-- Verify with :LspInfo inside a .fal buffer.

return {
	{
		"neovim/nvim-lspconfig",
		opts = function(_, opts)
			opts.servers = opts.servers or {}

			-- Register server definition (not yet in upstream nvim-lspconfig)
			local ok, configs = pcall(require, "lspconfig.configs")
			if ok and not configs.falcon_lsp then
				local lspconfig = require("lspconfig")
				configs.falcon_lsp = {
					default_config = {
						cmd = { "/opt/falcon/lib/falcon-lsp" },
						filetypes = { "falcon" },
						root_dir = lspconfig.util.root_pattern(".git", "*.fal"),
						settings = {},
						init_options = {},
					},
				}
			end

			opts.servers.falcon_lsp = {}
		end,

		init = function()
			vim.api.nvim_create_autocmd("LspAttach", {
				group = vim.api.nvim_create_augroup("falcon_lsp_keymaps", { clear = true }),
				callback = function(args)
					local client = vim.lsp.get_client_by_id(args.data.client_id)
					if not client or client.name ~= "falcon_lsp" then
						return
					end
					local buf = args.buf
					local o = { buffer = buf, silent = true }
					vim.keymap.set("n", "K", vim.lsp.buf.hover, o)
					vim.keymap.set("n", "gd", vim.lsp.buf.definition, o)
					vim.keymap.set("n", "<C-Space>", vim.lsp.buf.completion, o)
					vim.keymap.set("n", "<leader>rn", vim.lsp.buf.rename, o)
					vim.keymap.set("n", "<leader>e", vim.diagnostic.open_float, o)
				end,
			})
		end,
	},
}
