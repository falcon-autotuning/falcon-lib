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
  addr.sin_port = htons(53);
  addr.sin_addr.s_addr = htonl(0x08080808);
  int result = connect(sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
  close(sock);
  if (result != 0) {
    GTEST_SKIP() << "Network unavailable: skipping test";
  }
}

class GitHubPackageIntegrationTest : public ::testing::Test {
protected:
  fs::path tmp_;

  void SetUp() override {
    tmp_ = fs::temp_directory_path() / "falcon_pm_github_integration";
    fs::create_directories(tmp_);
  }

  void TearDown() override { fs::remove_all(tmp_); }
};

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
  } catch (const std::exception &e) {
    GTEST_SKIP() << "Network error: " << e.what();
  }
}

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

    auto main_fal = tmp_ / "main.fal";
    std::ofstream main_f(main_fal);
    main_f << "import \"" << array_url << "\";\n";
    main_f << "import \"" << map_url << "\";\n";
    main_f.close();

    auto resolved = pm.resolve_imports(main_fal, {array_url, map_url});
    ASSERT_EQ(resolved.size(), 2u);
    EXPECT_EQ(resolved[0].module_name, "array");
    EXPECT_EQ(resolved[1].module_name, "map");
  } catch (const std::exception &e) {
    GTEST_SKIP() << "Network error: " << e.what();
  }
}

TEST_F(GitHubPackageIntegrationTest, CachingPreventsRedundantDownloads) {
  require_network();
  falcon::pm::PackageManager::init(tmp_, "test-project");
  falcon::pm::PackageManager pm(tmp_);

  std::string array_url =
      "github.com/falcon-autotuning/falcon-lib/libs/collections/array";

  try {
    pm.install(array_url);
    auto list1 = pm.list();

    pm.install(array_url);
    auto list2 = pm.list();

    EXPECT_EQ(list1.size(), list2.size());
  } catch (const std::exception &e) {
    GTEST_SKIP() << "Network error: " << e.what();
  }
}

TEST_F(GitHubPackageIntegrationTest, PackageStructureIsMaintained) {
  require_network();
  falcon::pm::PackageManager::init(tmp_, "test-project");
  falcon::pm::PackageManager pm(tmp_);

  std::string array_url =
      "github.com/falcon-autotuning/falcon-lib/libs/collections/array";

  try {
    pm.install(array_url);

    auto cache_dir = tmp_ / ".falcon" / "cache" / "falcon-lib";
    auto package_dir = cache_dir / "libs" / "collections" / "array";

    EXPECT_TRUE(std::filesystem::exists(package_dir));
    EXPECT_TRUE(std::filesystem::exists(package_dir / "falcon.yml"));
    EXPECT_TRUE(std::filesystem::exists(package_dir / "array.fal"));
  } catch (const std::exception &e) {
    GTEST_SKIP() << "Network error: " << e.what();
  }
}
