#include "falcon-pm/PackageCache.hpp"
#include "falcon-pm/PackageResolver.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

namespace fs = std::filesystem;

class ResolverTest : public ::testing::Test {
protected:
  fs::path tmp_;
  void SetUp() override {
    tmp_ = fs::temp_directory_path() / "falcon_pm_resolver_test";
    fs::create_directories(tmp_ / "src");
    fs::create_directories(tmp_ / "src" / "sub");
  }
  void TearDown() override { fs::remove_all(tmp_); }

  fs::path write_fal(const fs::path &rel,
                     const std::string &content = "routine foo(){}") {
    auto p = tmp_ / rel;
    std::ofstream f(p);
    f << content;
    return p;
  }
};

TEST_F(ResolverTest, ResolveRelativeLocalFile) {
  auto adder = write_fal("src/Adder.fal",
                         "routine adder(int a, int b) -> (int r){ r = a+b; }");
  falcon::pm::PackageCache cache(tmp_ / ".cache");
  falcon::pm::PackageResolver resolver(tmp_, cache);

  auto importing = tmp_ / "src" / "main.fal";
  auto resolved = resolver.resolve("./Adder.fal", importing);

  EXPECT_EQ(fs::weakly_canonical(resolved.absolute_path),
            fs::weakly_canonical(adder));
  EXPECT_EQ(resolved.module_name, "Adder");
  EXPECT_FALSE(resolved.sha256.empty());
}

TEST_F(ResolverTest, ResolveSubdirRelative) {
  auto lib = write_fal("src/sub/Lib.fal");
  falcon::pm::PackageCache cache(tmp_ / ".cache");
  falcon::pm::PackageResolver resolver(tmp_, cache);

  auto importing = tmp_ / "src" / "main.fal";
  auto resolved = resolver.resolve("./sub/Lib.fal", importing);
  EXPECT_EQ(resolved.module_name, "Lib");
}

TEST_F(ResolverTest, MissingFileThrows) {
  falcon::pm::PackageCache cache(tmp_ / ".cache");
  falcon::pm::PackageResolver resolver(tmp_, cache);

  auto importing = tmp_ / "src" / "main.fal";
  EXPECT_THROW(resolver.resolve("./NonExistent.fal", importing),
               std::runtime_error);
}

TEST_F(ResolverTest, ResolveAllMultiple) {
  write_fal("src/Adder.fal");
  write_fal("src/Multiplier.fal");
  falcon::pm::PackageCache cache(tmp_ / ".cache");
  falcon::pm::PackageResolver resolver(tmp_, cache);

  auto importing = tmp_ / "src" / "main.fal";
  auto results =
      resolver.resolve_all({"./Adder.fal", "./Multiplier.fal"}, importing);
  ASSERT_EQ(results.size(), 2u);
  EXPECT_EQ(results[0].module_name, "Adder");
  EXPECT_EQ(results[1].module_name, "Multiplier");
}
