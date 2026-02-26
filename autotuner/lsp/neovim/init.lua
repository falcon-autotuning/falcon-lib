-- Example lazy.nvim configuration for falcon-lsp
--
-- Add this file (or its contents) to your lazy.nvim plugin specs.

return {
  {
    'neovim/nvim-lspconfig',
    config = function()
      require('falcon_lsp').setup({
        on_attach = function(client, bufnr)
          local opts = { buffer = bufnr, silent = true }
          vim.keymap.set('n', 'K',          vim.lsp.buf.hover,       opts)
          vim.keymap.set('n', 'gd',         vim.lsp.buf.definition,  opts)
          vim.keymap.set('n', '<C-Space>',  vim.lsp.buf.completion,  opts)
          vim.keymap.set('n', '<leader>rn', vim.lsp.buf.rename,      opts)
          vim.keymap.set('n', '<leader>e',  vim.diagnostic.open_float, opts)
        end,
      })
    end,
  },
}
