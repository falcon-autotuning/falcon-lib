#include "falcon-pm/PackageCache.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

namespace fs = std::filesystem;

class CacheTest : public ::testing::Test {
protected:
  fs::path tmp_;
  void SetUp() override {
    tmp_ = fs::temp_directory_path() / "falcon_pm_cache_test";
    fs::create_directories(tmp_);
  }
  void TearDown() override { fs::remove_all(tmp_); }

  fs::path write_file(const std::string &name, const std::string &content) {
    auto p = tmp_ / name;
    std::ofstream f(p);
    f << content;
    return p;
  }
};

TEST_F(CacheTest, Sha256IsStable) {
  auto s1 = falcon::pm::PackageCache::sha256_string("hello");
  auto s2 = falcon::pm::PackageCache::sha256_string("hello");
  EXPECT_EQ(s1, s2);
  EXPECT_EQ(s1.size(), 64u); // 32 bytes hex
}

TEST_F(CacheTest, Sha256DiffersForDifferentContent) {
  auto s1 = falcon::pm::PackageCache::sha256_string("hello");
  auto s2 = falcon::pm::PackageCache::sha256_string("world");
  EXPECT_NE(s1, s2);
}

TEST_F(CacheTest, FileHashingMatchesString) {
  auto src = write_file("test.fal", "routine foo() -> (int r) { r = 1; }");

  auto file_hash = falcon::pm::PackageCache::sha256_file(src);
  auto string_hash = falcon::pm::PackageCache::sha256_string(
      "routine foo() -> (int r) { r = 1; }");

  EXPECT_EQ(file_hash, string_hash);
}
