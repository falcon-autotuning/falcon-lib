#include <filesystem>
#include <gtest/gtest.h>
#include <cstdlib>

namespace fs = std::filesystem;

#ifndef FALCON_LSP_SOURCE_DIR
#define FALCON_LSP_SOURCE_DIR "."
#endif

#ifndef FALCON_LINT_BIN
#define FALCON_LINT_BIN "falcon-lint"
#endif

#ifndef TEST_AUTOTUNERS_DIR
#define TEST_AUTOTUNERS_DIR "test-autotuners"
#endif

TEST(TreeSitterIntegration, GrammarFileExists) {
    fs::path grammar = fs::path(FALCON_LSP_SOURCE_DIR) / "treesitter" / "grammar.js";
    EXPECT_TRUE(fs::exists(grammar)) << "grammar.js not found at " << grammar;
}

TEST(TreeSitterIntegration, NeovimConfigExists) {
    fs::path lua = fs::path(FALCON_LSP_SOURCE_DIR) / "neovim" / "falcon_lsp.lua";
    EXPECT_TRUE(fs::exists(lua)) << "falcon_lsp.lua not found at " << lua;
}

TEST(TreeSitterIntegration, NeovimInitExists) {
    fs::path init = fs::path(FALCON_LSP_SOURCE_DIR) / "neovim" / "init.lua";
    EXPECT_TRUE(fs::exists(init)) << "init.lua not found at " << init;
}

TEST(TreeSitterIntegration, LinterPassesOnValidFile) {
    std::string cmd = std::string(FALCON_LINT_BIN) + " "
                    + std::string(TEST_AUTOTUNERS_DIR)
                    + "/basic_features/calculator.fal";
    int ret = std::system(cmd.c_str());
    EXPECT_EQ(0, WEXITSTATUS(ret));
}

TEST(TreeSitterIntegration, LinterFailsOnInvalidFile) {
    // simple-error.fal is intentionally broken
    std::string cmd = std::string(FALCON_LINT_BIN) + " "
                    + std::string(TEST_AUTOTUNERS_DIR)
                    + "/simple-error.fal 2>/dev/null";
    int ret = std::system(cmd.c_str());
    EXPECT_NE(0, WEXITSTATUS(ret));
}
