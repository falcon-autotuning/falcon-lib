-- Configuration for falcon-lsp in Neovim
local M = {}

function M.setup(opts)
  opts = opts or {}
  local lspconfig = require('lspconfig')
  local configs = require('lspconfig.configs')

  if not configs.falcon_lsp then
    configs.falcon_lsp = {
      default_config = {
        cmd = { 'falcon-lsp' },
        filetypes = { 'fal' },
        root_dir = lspconfig.util.root_pattern('.git', '*.fal'),
        settings = {},
      },
    }
  end

  lspconfig.falcon_lsp.setup(opts)
end

-- Filetype detection for .fal files
vim.filetype.add({ extension = { fal = 'fal' } })

return M
