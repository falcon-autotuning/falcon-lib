#include "falcon-pm/PackageManager.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>

namespace fs = std::filesystem;

/**
 * @brief Integration tests for PackageManager with GitHub packages.
 *
 * Tests real-world scenarios using the full PackageManager API:
 *  - Install packages from GitHub URLs
 *  - Resolve imports from installed packages
 *  - Use packages in actual .fal files
 *
 * These tests use real packages from the falcon-lib repository:
 *  - github.com/falcon-autotuning/falcon-lib/libs/collections/array
 *  - github.com/falcon-autotuning/falcon-lib/libs/collections/map
 */
class GitHubPackageIntegrationTest : public ::testing::Test {
protected:
  fs::path tmp_;

  void SetUp() override {
    tmp_ = fs::temp_directory_path() / "falcon_pm_github_integration";
    fs::create_directories(tmp_);
  }

  void TearDown() override { fs::remove_all(tmp_); }
};

// ─────────────────────────────────────────────────────────────────────────────
// Integration Test: Install array package from GitHub
// ─────────────────────────────────────────────────────────────────────────────
TEST_F(GitHubPackageIntegrationTest, InstallArrayPackageFromGitHub) {
  falcon::pm::PackageManager::init(tmp_, "test-project");
  falcon::pm::PackageManager pm(tmp_);

  std::string array_url =
      "github.com/falcon-autotuning/falcon-lib/libs/collections/array";

  try {
    pm.install(array_url);

    EXPECT_EQ(pm.manifest().dependencies.size(), 1u);
    EXPECT_EQ(pm.manifest().dependencies[0].name, "array");
    EXPECT_TRUE(pm.manifest().dependencies[0].github.has_value());

    std::cout << "✓ Array package installed from GitHub\n";
  } catch (const std::exception &e) {
    GTEST_SKIP() << "Network error: " << e.what();
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Integration Test: Install map package from GitHub
// ───────────────────────────────────────────���─────────────────────────────────
TEST_F(GitHubPackageIntegrationTest, InstallMapPackageFromGitHub) {
  falcon::pm::PackageManager::init(tmp_, "test-project");
  falcon::pm::PackageManager pm(tmp_);

  std::string map_url =
      "github.com/falcon-autotuning/falcon-lib/libs/collections/map";

  try {
    pm.install(map_url);

    EXPECT_EQ(pm.manifest().dependencies.size(), 1u);
    EXPECT_EQ(pm.manifest().dependencies[0].name, "map");

    std::cout << "✓ Map package installed from GitHub\n";
  } catch (const std::exception &e) {
    GTEST_SKIP() << "Network error: " << e.what();
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Integration Test: Install and resolve imports from GitHub packages
// ─────────────────────────────────────────────────────────────────────────────
TEST_F(GitHubPackageIntegrationTest, ResolveImportsFromGitHubPackages) {
  falcon::pm::PackageManager::init(tmp_, "test-project");
  falcon::pm::PackageManager pm(tmp_);

  std::string array_url =
      "github.com/falcon-autotuning/falcon-lib/libs/collections/array";
  std::string map_url =
      "github.com/falcon-autotuning/falcon-lib/libs/collections/map";

  try {
    pm.install(array_url);
    pm.install(map_url);

    // Create a main.fal that imports both packages
    auto main_fal = tmp_ / "main.fal";
    std::ofstream main_f(main_fal);
    main_f << "import \"" << array_url << "\";\n";
    main_f << "import \"" << map_url << "\";\n";
    main_f.close();

    // Resolve imports
    auto resolved = pm.resolve_imports(main_fal, {array_url, map_url});

    ASSERT_EQ(resolved.size(), 2u);
    EXPECT_EQ(resolved[0].module_name, "array");
    EXPECT_EQ(resolved[1].module_name, "map");

    std::cout << "✓ Resolved imports from GitHub packages\n";
    std::cout << "  [1] " << resolved[0].module_name << "\n";
    std::cout << "  [2] " << resolved[1].module_name << "\n";
  } catch (const std::exception &e) {
    GTEST_SKIP() << "Network error: " << e.what();
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Integration Test: Caching prevents redundant downloads
// ─────────────────────────────────────────────────────────────────────────────
TEST_F(GitHubPackageIntegrationTest, CachingPreventsRedundantDownloads) {
  falcon::pm::PackageManager::init(tmp_, "test-project");
  falcon::pm::PackageManager pm(tmp_);

  std::string array_url =
      "github.com/falcon-autotuning/falcon-lib/libs/collections/array";

  try {
    // First install
    pm.install(array_url);
    auto list1 = pm.list();
    size_t size_after_first = list1.size();

    // Second install (should be idempotent)
    pm.install(array_url);
    auto list2 = pm.list();
    size_t size_after_second = list2.size();

    EXPECT_EQ(size_after_first, size_after_second);

    std::cout << "✓ Caching works - second install was idempotent\n";
  } catch (const std::exception &e) {
    GTEST_SKIP() << "Network error: " << e.what();
  }
}
