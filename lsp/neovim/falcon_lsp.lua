-- falcon-lsp Neovim configuration
-- Registers the falcon-lsp language server with nvim-lspconfig.
--
-- Usage (lazy.nvim):
--   require('falcon_lsp').setup({ on_attach = my_on_attach, capabilities = my_caps })

local M = {}

function M.setup(opts)
  opts = opts or {}

  -- Register .fal filetype
  vim.filetype.add({ extension = { fal = 'fal' } })

  local ok_lspconfig, lspconfig = pcall(require, 'lspconfig')
  if not ok_lspconfig then
    vim.notify('falcon_lsp: nvim-lspconfig not found', vim.log.levels.ERROR)
    return
  end

  local configs = require('lspconfig.configs')

  if not configs.falcon_lsp then
    configs.falcon_lsp = {
      default_config = {
        cmd        = { 'falcon-lsp' },
        filetypes  = { 'fal' },
        root_dir   = lspconfig.util.root_pattern('.git', '*.fal'),
        settings   = {},
        init_options = {},
      },
    }
  end

  lspconfig.falcon_lsp.setup(vim.tbl_extend('force', {
    on_attach    = opts.on_attach,
    capabilities = opts.capabilities,
  }, opts.server or {}))
end

return M
