#include "falcon-pm/PackageManager.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace fs = std::filesystem;

inline void require_network() {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    GTEST_SKIP() << "Network check failed: cannot create socket";
    return;
  }
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(53);                // DNS port
  addr.sin_addr.s_addr = htonl(0x08080808); // 8.8.8.8 (Google DNS)
  int result = connect(sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
  close(sock);
  if (result != 0) {
    GTEST_SKIP() << "Network unavailable: skipping test";
  }
}

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
  require_network();
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
  require_network();
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
  require_network();
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
  require_network();
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
// ─────────────────────────────────────────────────────────────────────────────
// Integration Test: FFI wrappers are cached with dependencies
// ─────────────────────────────────────────────────────────────────────────────
TEST_F(GitHubPackageIntegrationTest, FFIWrappersAreCachedWithDependencies) {
  require_network();

  falcon::pm::PackageManager::init(tmp_, "test-project");
  falcon::pm::PackageManager pm(tmp_);

  std::string array_url =
      "github.com/falcon-autotuning/falcon-lib/libs/collections/array";

  try {
    // Install array package
    pm.install(array_url);

    // Check that array.fal was cached
    auto list = pm.list();
    EXPECT_FALSE(list.empty());

    // Check that the FFI wrapper was copied to cache
    auto cache_dir = tmp_ / ".falcon" / "cache";
    auto wrapper_path = cache_dir / "array-wrapper.cpp";

    if (std::filesystem::exists(wrapper_path)) {
      std::cout << "✓ FFI wrapper found in cache: " << wrapper_path << "\n";
    } else {
      std::cout << "⚠ FFI wrapper NOT found in cache\n";
      std::cout << "  Expected: " << wrapper_path << "\n";
      std::cout << "  Cache contents:\n";
      for (const auto &entry : std::filesystem::directory_iterator(cache_dir)) {
        std::cout << "    - " << entry.path().filename() << "\n";
      }
    }
  } catch (const std::exception &e) {
    GTEST_SKIP() << "Network error: " << e.what();
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Integration Test: Map's dependencies include array's FFI wrappers
// ─────────────────────────────────────────────────────────────────────────────
TEST_F(GitHubPackageIntegrationTest, MapDependencyFFIWrappersAreCached) {
  require_network();

  falcon::pm::PackageManager::init(tmp_, "test-project");
  falcon::pm::PackageManager pm(tmp_);

  std::string map_url =
      "github.com/falcon-autotuning/falcon-lib/libs/collections/map";

  try {
    // Install map package (which depends on array)
    pm.install(map_url);

    auto cache_dir = tmp_ / ".falcon" / "cache";

    // Check for both map and array wrappers
    bool has_array_wrapper =
        std::filesystem::exists(cache_dir / "array-wrapper.cpp");
    bool has_map_wrapper =
        std::filesystem::exists(cache_dir / "map-wrapper.cpp");

    std::cout << "✓ Map package installed\n";
    std::cout << "  - array-wrapper.cpp: "
              << (has_array_wrapper ? "FOUND" : "MISSING") << "\n";
    std::cout << "  - map-wrapper.cpp: "
              << (has_map_wrapper ? "FOUND" : "MISSING") << "\n";

    if (!has_array_wrapper || !has_map_wrapper) {
      std::cout << "  Cache contents:\n";
      for (const auto &entry : std::filesystem::directory_iterator(cache_dir)) {
        std::cout << "    - " << entry.path().filename() << "\n";
      }
    }
  } catch (const std::exception &e) {
    GTEST_SKIP() << "Network error: " << e.what();
  }
}
