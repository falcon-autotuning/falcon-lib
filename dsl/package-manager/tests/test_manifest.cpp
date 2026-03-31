#include "falcon-pm/PackageManifest.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

namespace fs = std::filesystem;

class ManifestTest : public ::testing::Test {
protected:
  fs::path tmp_;
  void SetUp() override {
    tmp_ = fs::temp_directory_path() / "falcon_pm_manifest_test";
    fs::create_directories(tmp_);
  }
  void TearDown() override { fs::remove_all(tmp_); }
};

TEST_F(ManifestTest, RoundTripEmpty) {
  auto m = falcon::pm::PackageManifest::make_empty("test-pkg");
  EXPECT_EQ(m.name, "test-pkg");
  EXPECT_EQ(m.version, "0.1.0");
  EXPECT_TRUE(m.dependencies.empty());

  auto path = tmp_ / "falcon.yml";
  m.save(path);
  auto m2 = falcon::pm::PackageManifest::load(path);
  EXPECT_EQ(m2.name, "test-pkg");
  EXPECT_EQ(m2.version, "0.1.0");
}

TEST_F(ManifestTest, RoundTripWithFFIMap) {
  falcon::pm::PackageManifest m =
      falcon::pm::PackageManifest::make_empty("test-ffi");
  m.ffi["wrapper.so"] = "sha256:abcdef123456";

  auto path = tmp_ / "falcon.yml";
  m.save(path);

  auto m2 = falcon::pm::PackageManifest::load(path);
  ASSERT_EQ(m2.ffi.size(), 1u);
  EXPECT_EQ(m2.ffi["wrapper.so"], "sha256:abcdef123456");
}

TEST_F(ManifestTest, FindRootFromSubdir) {
  auto sub = tmp_ / "a" / "b" / "c";
  fs::create_directories(sub);
  auto m = falcon::pm::PackageManifest::make_empty("root-pkg");
  m.save(tmp_ / "falcon.yml");

  auto found = falcon::pm::PackageManifest::find_root(sub);
  ASSERT_TRUE(found.has_value());
  EXPECT_EQ(fs::weakly_canonical(*found), fs::weakly_canonical(tmp_));
}
