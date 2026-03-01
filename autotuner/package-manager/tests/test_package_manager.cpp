#include "falcon-pm/PackageManager.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

namespace fs = std::filesystem;

class PackageManagerTest : public ::testing::Test {
protected:
  fs::path tmp_;
  void SetUp() override {
    tmp_ = fs::temp_directory_path() / "falcon_pm_manager_test";
    fs::create_directories(tmp_);
  }
  void TearDown() override { fs::remove_all(tmp_); }

  fs::path write_fal(const std::string &name,
                     const std::string &content = "routine foo(){}") {
    auto p = tmp_ / name;
    std::ofstream f(p);
    f << content;
    return p;
  }
};

TEST_F(PackageManagerTest, InitCreatesManifestAndCache) {
  falcon::pm::PackageManager::init(tmp_, "my-project");
  EXPECT_TRUE(fs::exists(tmp_ / "falcon.yml"));
  EXPECT_TRUE(fs::exists(tmp_ / ".falcon" / "cache"));
}

TEST_F(PackageManagerTest, InitThrowsIfAlreadyExists) {
  falcon::pm::PackageManager::init(tmp_, "my-project");
  EXPECT_THROW(falcon::pm::PackageManager::init(tmp_, "my-project"),
               std::runtime_error);
}

TEST_F(PackageManagerTest, ResolveImportsLocalFile) {
  falcon::pm::PackageManager::init(tmp_, "my-project");
  auto adder = write_fal("Adder.fal",
                         "routine adder(int a, int b) -> (int r){ r = a+b; }");
  auto main_fal = write_fal("main.fal");

  falcon::pm::PackageManager pm(tmp_);
  auto resolved = pm.resolve_imports(main_fal, {"./Adder.fal"});
  ASSERT_EQ(resolved.size(), 1u);
  EXPECT_EQ(resolved[0].module_name, "Adder");
}

TEST_F(PackageManagerTest, InstallAndList) {
  falcon::pm::PackageManager::init(tmp_, "my-project");
  auto adder = write_fal("Adder.fal");

  falcon::pm::PackageManager pm(tmp_);
  pm.install(adder.string());

  auto list = pm.list();
  EXPECT_FALSE(list.empty());
}

TEST_F(PackageManagerTest, RemovePackage) {
  falcon::pm::PackageManager::init(tmp_, "my-project");
  auto adder = write_fal("Adder.fal");

  falcon::pm::PackageManager pm(tmp_);
  pm.install(adder.string());

  EXPECT_NO_THROW(pm.remove("Adder"));
  // After removal the manifest dep should be gone
  EXPECT_TRUE(pm.manifest().dependencies.empty());
}
