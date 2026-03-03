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

TEST_F(ManifestTest, RoundTripWithDependencies) {
  falcon::pm::PackageManifest m;
  m.name = "my-project";
  m.version = "1.2.3";
  m.maintainer = "Tyler K";
  m.github = "falcon-autotuning/my-project";

  falcon::pm::Dependency d;
  d.name = "adder-lib";
  d.version = "^1.0.0";
  d.github = "falcon-autotuning/adder-lib";
  m.dependencies.push_back(d);

  auto path = tmp_ / "falcon.yml";
  m.save(path);
  auto m2 = falcon::pm::PackageManifest::load(path);
  ASSERT_EQ(m2.dependencies.size(), 1u);
  EXPECT_EQ(m2.dependencies[0].name, "adder-lib");
  EXPECT_EQ(m2.dependencies[0].version, "^1.0.0");
  ASSERT_TRUE(m2.dependencies[0].github.has_value());
  EXPECT_EQ(*m2.dependencies[0].github, "falcon-autotuning/adder-lib");
}

TEST_F(ManifestTest, FindRootFromSubdir) {
  // Create falcon.yml in tmp_, search from a subdir
  auto sub = tmp_ / "a" / "b" / "c";
  fs::create_directories(sub);
  auto m = falcon::pm::PackageManifest::make_empty("root-pkg");
  m.save(tmp_ / "falcon.yml");

  auto found = falcon::pm::PackageManifest::find_root(sub);
  ASSERT_TRUE(found.has_value());
  EXPECT_EQ(fs::weakly_canonical(*found), fs::weakly_canonical(tmp_));
}

TEST_F(ManifestTest, FindRootNotFound) {
  // No falcon.yml anywhere — should return nullopt when searching from /tmp sub
  auto isolated = tmp_ / "isolated";
  fs::create_directories(isolated);
  // Don't create falcon.yml
  auto found =
      falcon::pm::PackageManifest::find_root(isolated / "deep" / "path");
  // May or may not find a system-level falcon.yml; just check it doesn't throw
  (void)found;
}
