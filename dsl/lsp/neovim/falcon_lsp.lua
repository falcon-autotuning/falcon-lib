-- Falcon LSP — LazyVim plugin spec
-- Install via:  cd dsl/lsp && make nvim-install
-- Or manually copy to ~/.config/nvim/lua/plugins/falcon_lsp.lua
--
-- Also copy neovim/ftdetect/falcon.vim to ~/.config/nvim/ftdetect/falcon.vim
-- (make nvim-install does both automatically)

return {
	{
		"neovim/nvim-lspconfig",
		opts = function(_, opts)
			opts.servers = opts.servers or {}

			-- Register the custom server definition (not yet in upstream nvim-lspconfig)
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

			-- LazyVim will call lspconfig.falcon_lsp.setup() with merged capabilities
			opts.servers.falcon_lsp = {}
		end,

		init = function()
			-- Wire keymaps when falcon_lsp attaches to a buffer
			vim.api.nvim_create_autocmd("LspAttach", {
				group = vim.api.nvim_create_augroup("falcon_lsp_keymaps", { clear = true }),
				callback = function(args)
					local client = vim.lsp.get_client_by_id(args.data.client_id)
					if not client or client.name ~= "falcon_lsp" then
						return
					end
					local buf = args.buf
					local opts = { buffer = buf, silent = true }
					vim.keymap.set("n", "K", vim.lsp.buf.hover, opts)
					vim.keymap.set("n", "gd", vim.lsp.buf.definition, opts)
					vim.keymap.set("n", "<C-Space>", vim.lsp.buf.completion, opts)
					vim.keymap.set("n", "<leader>rn", vim.lsp.buf.rename, opts)
					vim.keymap.set("n", "<leader>e", vim.diagnostic.open_float, opts)
				end,
			})
		end,
	},
}
