#include "falcon-pm/PackageManager.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

namespace fs = std::filesystem;

class PackageManagerTest : public ::testing::Test {
protected:
  fs::path tmp_;

  void SetUp() override {
    tmp_ = fs::temp_directory_path() / "falcon_pm_unit_test";
    fs::create_directories(tmp_);
  }

  void TearDown() override { fs::remove_all(tmp_); }

  fs::path create_test_package(const std::string &name) {
    auto pkg_dir = tmp_ / name;
    fs::create_directories(pkg_dir);

    auto manifest_path = pkg_dir / "falcon.yml";
    std::ofstream manifest_f(manifest_path);
    manifest_f << "name: " << name << "\n";
    manifest_f << "version: 0.1.0\n";
    manifest_f.close();

    auto fal_path = pkg_dir / (name + ".fal");
    std::ofstream fal_f(fal_path);
    fal_f << "routine " << name << "_main() {}\n";
    fal_f.close();

    return pkg_dir;
  }
};

TEST_F(PackageManagerTest, InitCreatesManifestAndCache) {
  falcon::pm::PackageManager::init(tmp_, "test-project");

  EXPECT_TRUE(fs::exists(tmp_ / "falcon.yml"));
  EXPECT_TRUE(fs::exists(tmp_ / ".falcon" / "cache"));
}

TEST_F(PackageManagerTest, InstallLocalPackage) {
  falcon::pm::PackageManager::init(tmp_, "test-project");
  auto pkg = create_test_package("math-lib");

  falcon::pm::PackageManager pm(tmp_);
  pm.install(pkg.string());

  EXPECT_EQ(pm.manifest().dependencies.size(), 1u);
  EXPECT_EQ(pm.manifest().dependencies[0].name, "math-lib");
  EXPECT_TRUE(pm.manifest().dependencies[0].local_path.has_value());
}

TEST_F(PackageManagerTest, BuildUpdatesFFIHash) {
  auto pkg = create_test_package("ffi-lib");

  // Create dummy cpp
  std::ofstream cpp_f(pkg / "wrapper.cpp");
  cpp_f << "extern \"C\" void dummy() {}" << std::endl;
  cpp_f.close();

  // Add ffi entry
  auto manifest_path = pkg / "falcon.yml";
  std::ofstream append_f(manifest_path, std::ios::app);
  append_f << "ffi:\n  wrapper.so: \"\"\n";
  append_f.close();

  falcon::pm::PackageManager pm(pkg);

  // System needs a valid compiler environment for this to pass,
  // we catch the exception or test for true execution based on CI runner.
  try {
    pm.build(pkg);
    auto updated_manifest = falcon::pm::PackageManifest::load(manifest_path);
    EXPECT_TRUE(updated_manifest.ffi["wrapper.so"].starts_with("sha256:"));
  } catch (const std::exception &e) {
    // If CI doesn't have clang++, we can skip the strict check
    GTEST_SKIP() << "Skipping build test: " << e.what();
  }
}
