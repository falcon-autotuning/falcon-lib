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

TEST_F(CacheTest, StoreAndLookup) {
  auto src = write_file("test.fal", "routine foo() -> (int r) { r = 1; }");
  falcon::pm::PackageCache cache(tmp_ / ".cache");

  auto cached = cache.store(src);
  EXPECT_TRUE(fs::exists(cached));

  auto result = cache.lookup(src);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, cached);
}

TEST_F(CacheTest, StaleAfterModification) {
  auto src = write_file("test.fal", "routine foo() -> (int r) { r = 1; }");
  falcon::pm::PackageCache cache(tmp_ / ".cache");
  cache.store(src);

  // Modify the source file — cache should become stale
  std::ofstream f(src, std::ios::app);
  f << "\n// modified";
  f.close();

  auto result = cache.lookup(src);
  EXPECT_FALSE(result.has_value());
}

TEST_F(CacheTest, InvalidateRemovesEntry) {
  auto src = write_file("test.fal", "routine foo() -> (int r) { r = 1; }");
  falcon::pm::PackageCache cache(tmp_ / ".cache");
  cache.store(src);
  EXPECT_TRUE(cache.lookup(src).has_value());

  cache.invalidate(src);
  EXPECT_FALSE(cache.lookup(src).has_value());
}

TEST_F(CacheTest, ClearRemovesAll) {
  auto f1 = write_file("a.fal", "routine a(){}");
  auto f2 = write_file("b.fal", "routine b(){}");
  falcon::pm::PackageCache cache(tmp_ / ".cache");
  cache.store(f1);
  cache.store(f2);
  cache.clear();
  EXPECT_FALSE(cache.lookup(f1).has_value());
  EXPECT_FALSE(cache.lookup(f2).has_value());
}
