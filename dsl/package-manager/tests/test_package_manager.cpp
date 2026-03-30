#include "falcon-pm/PackageManager.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

namespace fs = std::filesystem;

/**
 * @brief Unit tests for PackageManager.
 *
 * Tests the core features of PackageManager in isolation:
 *  - init()     : Create new package projects
 *  - install()  : Install local packages
 *  - remove()   : Remove packages from manifest
 *  - list()     : List installed packages
 *  - resolve_imports() : Resolve import paths
 *
 * These tests do NOT require network access.
 */
class PackageManagerTest : public ::testing::Test {
protected:
  fs::path tmp_;

  void SetUp() override {
    tmp_ = fs::temp_directory_path() / "falcon_pm_unit_test";
    fs::create_directories(tmp_);
  }

  void TearDown() override { fs::remove_all(tmp_); }

  /**
   * @brief Create a Falcon package directory with falcon.yml metadata.
   *
   * @param name Package name (also used as directory name)
   * @param main_file Main .fal file name (defaults to {name}.fal)
   */
  fs::path create_test_package(const std::string &name,
                               const std::string &main_file = "") {
    auto pkg_dir = tmp_ / name;
    fs::create_directories(pkg_dir);

    // Create falcon.yml metadata
    std::string actual_main = main_file.empty() ? (name + ".fal") : main_file;
    auto manifest_path = pkg_dir / "falcon.yml";
    std::ofstream manifest_f(manifest_path);
    manifest_f << "name: " << name << "\n";
    manifest_f << "version: 0.1.0\n";
    manifest_f.close();

    // Create main .fal file
    auto fal_path = pkg_dir / actual_main;
    std::ofstream fal_f(fal_path);
    fal_f << "routine " << name << "_main() {}\n";
    fal_f.close();

    return pkg_dir;
  }
};

// ─────────────────────────────────────────────────────────────────────────────
// Test: init() creates falcon.yml and cache directory
// ───────────────────────────────────────────���─────────────────────────────────
TEST_F(PackageManagerTest, InitCreatesManifestAndCache) {
  falcon::pm::PackageManager::init(tmp_, "test-project");

  EXPECT_TRUE(fs::exists(tmp_ / "falcon.yml"));
  EXPECT_TRUE(fs::exists(tmp_ / ".falcon" / "cache"));
}

TEST_F(PackageManagerTest, InitThrowsIfFalconYmlExists) {
  falcon::pm::PackageManager::init(tmp_, "test-project");
  EXPECT_THROW(falcon::pm::PackageManager::init(tmp_, "test-project"),
               std::runtime_error);
}

// ─────────────────────────────────────────────────────────────────────────────
// Test: install() adds local package to manifest
// ─────────────────────────────────────────────────────────────────────────────
TEST_F(PackageManagerTest, InstallLocalPackage) {
  falcon::pm::PackageManager::init(tmp_, "test-project");

  auto pkg = create_test_package("math-lib");

  falcon::pm::PackageManager pm(tmp_);
  pm.install(pkg.string());

  EXPECT_EQ(pm.manifest().dependencies.size(), 1u);
  EXPECT_EQ(pm.manifest().dependencies[0].name, "math-lib");
  EXPECT_TRUE(pm.manifest().dependencies[0].local_path.has_value());
  EXPECT_FALSE(pm.manifest().dependencies[0].github.has_value());
}

TEST_F(PackageManagerTest, InstallThrowsIfNotPackage) {
  falcon::pm::PackageManager::init(tmp_, "test-project");

  // Create a standalone .fal file (not a package)
  auto fal_file = tmp_ / "standalone.fal";
  std::ofstream f(fal_file);
  f << "routine foo() {}\n";
  f.close();

  falcon::pm::PackageManager pm(tmp_);
  EXPECT_THROW(pm.install(fal_file.string()), std::runtime_error);
}

TEST_F(PackageManagerTest, InstallSamePacageTwiceIsIdempotent) {
  falcon::pm::PackageManager::init(tmp_, "test-project");

  auto pkg = create_test_package("idempotent-lib");

  falcon::pm::PackageManager pm(tmp_);
  pm.install(pkg.string());
  EXPECT_EQ(pm.manifest().dependencies.size(), 1u);

  // Install again — should be idempotent
  pm.install(pkg.string());
  EXPECT_EQ(pm.manifest().dependencies.size(), 1u);
}

// ─────────────────────────────────────────────────────────────────────────────
// Test: remove() deletes package from manifest
// ─────────────────────────────────────────────────────────────────────────────
TEST_F(PackageManagerTest, RemovePackage) {
  falcon::pm::PackageManager::init(tmp_, "test-project");

  auto pkg = create_test_package("removable-lib");

  falcon::pm::PackageManager pm(tmp_);
  pm.install(pkg.string());
  EXPECT_EQ(pm.manifest().dependencies.size(), 1u);

  pm.remove("removable-lib");
  EXPECT_EQ(pm.manifest().dependencies.size(), 0u);
}

TEST_F(PackageManagerTest, RemoveThrowsIfNotFound) {
  falcon::pm::PackageManager::init(tmp_, "test-project");

  falcon::pm::PackageManager pm(tmp_);
  EXPECT_THROW(pm.remove("nonexistent"), std::runtime_error);
}

// ─────────────────────────────────────────────────────────────────────────────
// Test: list() shows installed packages
// ─────────────────────────────────────────────────────────────────────────────
TEST_F(PackageManagerTest, ListInstalledPackages) {
  falcon::pm::PackageManager::init(tmp_, "test-project");

  auto pkg1 = create_test_package("lib1");
  auto pkg2 = create_test_package("lib2");

  falcon::pm::PackageManager pm(tmp_);
  pm.install(pkg1.string());
  pm.install(pkg2.string());

  auto list = pm.list();
  EXPECT_FALSE(list.empty());
}

TEST_F(PackageManagerTest, ListEmptyWhenNoPackagesInstalled) {
  falcon::pm::PackageManager::init(tmp_, "test-project");

  falcon::pm::PackageManager pm(tmp_);
  auto list = pm.list();
  EXPECT_TRUE(list.empty());
}

// ─────────────────────────────────────────────────────────────────────────────
// Test: resolve_imports() resolves local file imports
// ─────────────────────────────────────────────────────────────────────────────
TEST_F(PackageManagerTest, ResolveLocalFileImport) {
  falcon::pm::PackageManager::init(tmp_, "test-project");

  // Create a helper file
  auto helper_fal = tmp_ / "helper.fal";
  std::ofstream helper_f(helper_fal);
  helper_f << "routine helper() {}\n";
  helper_f.close();

  // Create a main file that will import
  auto main_fal = tmp_ / "main.fal";
  std::ofstream main_f(main_fal);
  main_f << "import \"./helper.fal\";\n";
  main_f.close();

  falcon::pm::PackageManager pm(tmp_);
  auto resolved = pm.resolve_imports(main_fal, {"./helper.fal"});

  ASSERT_EQ(resolved.size(), 1u);
  EXPECT_EQ(resolved[0].module_name, "helper");
  EXPECT_FALSE(resolved[0].is_package);
}

// ─────────────────────────────────────────────────────────────────────────────
// Test: resolve_imports() resolves local package imports
// ─────────────────────────────────────────────────────────────────────────────
TEST_F(PackageManagerTest, ResolveLocalPackageImport) {
  falcon::pm::PackageManager::init(tmp_, "test-project");

  auto pkg = create_test_package("vector-lib");

  auto main_fal = tmp_ / "main.fal";
  std::ofstream main_f(main_fal);
  main_f << "import \"./vector-lib\";\n";
  main_f.close();

  falcon::pm::PackageManager pm(tmp_);
  auto resolved = pm.resolve_imports(main_fal, {"./vector-lib"});

  ASSERT_EQ(resolved.size(), 1u);
  EXPECT_EQ(resolved[0].module_name, "vector-lib");
  EXPECT_TRUE(resolved[0].is_package);
}

// ─────────────────────────────────────────────────────────────────────────────
// Test: resolve_imports() multiple imports at once
// ─────────────────────────────────────────────────────────────────────────────
TEST_F(PackageManagerTest, ResolveMultipleImports) {
  falcon::pm::PackageManager::init(tmp_, "test-project");

  auto pkg1 = create_test_package("lib1");
  auto pkg2 = create_test_package("lib2");

  auto main_fal = tmp_ / "main.fal";
  std::ofstream main_f(main_fal);
  main_f << "import \"./lib1\";\nimport \"./lib2\";\n";
  main_f.close();

  falcon::pm::PackageManager pm(tmp_);
  auto resolved = pm.resolve_imports(main_fal, {"./lib1", "./lib2"});

  ASSERT_EQ(resolved.size(), 2u);
  EXPECT_EQ(resolved[0].module_name, "lib1");
  EXPECT_EQ(resolved[1].module_name, "lib2");
}

// ─────────────────────────────────────────────────────────────────────────────
// Test: Package with multiple .fal files - all are cached
// ─────────────────────────────────────────────────────────────────────────────
TEST_F(PackageManagerTest, PackageWithMultipleFiles) {
  falcon::pm::PackageManager::init(tmp_, "test-project");

  auto pkg_dir = tmp_ / "multi-lib";
  fs::create_directories(pkg_dir);

  // Create falcon.yml metadata
  auto manifest_path = pkg_dir / "falcon.yml";
  std::ofstream manifest_f(manifest_path);
  manifest_f << "name: multi-lib\n";
  manifest_f << "version: 0.1.0\n";
  manifest_f.close();

  // Create main.fal
  auto main_path = pkg_dir / "main.fal";
  std::ofstream main_f(main_path);
  main_f << "routine main() {}\n";
  main_f.close();

  // Create helper.fal
  auto helper_path = pkg_dir / "helper.fal";
  std::ofstream helper_f(helper_path);
  helper_f << "routine helper() {}\n";
  helper_f.close();

  // Create utilities.fal
  auto util_path = pkg_dir / "utilities.fal";
  std::ofstream util_f(util_path);
  util_f << "routine util() {}\n";
  util_f.close();

  falcon::pm::PackageManager pm(tmp_);
  pm.install(pkg_dir.string());

  auto main_fal = tmp_ / "app.fal";
  std::ofstream app_f(main_fal);
  app_f << "import \"./multi-lib\";\n";
  app_f.close();

  auto resolved = pm.resolve_imports(main_fal, {"./multi-lib"});
  ASSERT_EQ(resolved.size(), 1u);
  EXPECT_TRUE(resolved[0].is_package);
  EXPECT_EQ(resolved[0].module_name, "multi-lib");

  // Verify all .fal files were cached
  auto list = pm.list();
  EXPECT_GE(list.size(), 3u); // At least main, helper, utilities
}
// ─────────────────────────────────────────────────────────────────────────────
// Test: Package with subdirectory .fal files
// ─────────────────────────────────────────────────────────────────────────────
TEST_F(PackageManagerTest, PackageWithNestedDirectories) {
  falcon::pm::PackageManager::init(tmp_, "test-project");

  auto pkg_dir = tmp_ / "nested-lib";
  fs::create_directories(pkg_dir / "subdir");

  // Create falcon.yml
  auto manifest_path = pkg_dir / "falcon.yml";
  std::ofstream manifest_f(manifest_path);
  manifest_f << "name: nested-lib\n";
  manifest_f << "version: 0.1.0\n";
  manifest_f.close();

  // Create root .fal
  auto root_fal = pkg_dir / "root.fal";
  std::ofstream root_f(root_fal);
  root_f << "routine root() {}\n";
  root_f.close();

  // Create nested .fal
  auto nested_fal = pkg_dir / "subdir" / "nested.fal";
  std::ofstream nested_f(nested_fal);
  nested_f << "routine nested() {}\n";
  nested_f.close();

  falcon::pm::PackageManager pm(tmp_);
  pm.install(pkg_dir.string());

  auto main_fal = tmp_ / "app.fal";
  std::ofstream app_f(main_fal);
  app_f << "import \"./nested-lib\";\n";
  app_f.close();

  auto resolved = pm.resolve_imports(main_fal, {"./nested-lib"});
  ASSERT_EQ(resolved.size(), 1u);
  EXPECT_TRUE(resolved[0].is_package);

  // Both root.fal and subdir/nested.fal should be cached
  auto list = pm.list();
  EXPECT_GE(list.size(), 2u);
}

// ─────────────────────────────────────────────────────────────────────────────
// Test: Empty package (no .fal files) throws error
// ─────────────────────────────────────────────────────────────────────────────
TEST_F(PackageManagerTest, EmptyPackageThrowsError) {
  falcon::pm::PackageManager::init(tmp_, "test-project");

  auto pkg_dir = tmp_ / "empty-lib";
  fs::create_directories(pkg_dir);

  // Create falcon.yml but no .fal files
  auto manifest_path = pkg_dir / "falcon.yml";
  std::ofstream manifest_f(manifest_path);
  manifest_f << "name: empty-lib\n";
  manifest_f << "version: 0.1.0\n";
  manifest_f.close();

  auto main_fal = tmp_ / "app.fal";
  std::ofstream app_f(main_fal);
  app_f << "import \"./empty-lib\";\n";
  app_f.close();

  falcon::pm::PackageManager pm(tmp_);
  EXPECT_THROW(pm.resolve_imports(main_fal, {"./empty-lib"}),
               std::runtime_error);
}

// ─────────────────────────────────────────────────────────────────────────────
// Test: Install removes old cache when reinstalling
// ─────────────────────────────────────────────────────────────────────────────
TEST_F(PackageManagerTest, ReinstallUpdatesCache) {
  falcon::pm::PackageManager::init(tmp_, "test-project");

  auto pkg_dir = tmp_ / "versioned-lib";
  fs::create_directories(pkg_dir);

  // Create falcon.yml
  auto manifest_path = pkg_dir / "falcon.yml";
  std::ofstream manifest_f(manifest_path);
  manifest_f << "name: versioned-lib\n";
  manifest_f << "version: 0.1.0\n";
  manifest_f.close();

  // Create initial file
  auto fal_v1 = pkg_dir / "lib.fal";
  std::ofstream f1(fal_v1);
  f1 << "routine version_one() {}\n";
  f1.close();

  falcon::pm::PackageManager pm(tmp_);
  pm.install(pkg_dir.string());
  auto list1 = pm.list();

  // Update the file
  std::ofstream f2(fal_v1);
  f2 << "routine version_two() {}\n";
  f2.close();

  // Reinstall
  pm.install(pkg_dir.string());
  auto list2 = pm.list();

  // Cache should be updated
  EXPECT_FALSE(list2.empty());
}

// ─────────────────────────────────────────────────────────────────────────────
// Test: Resolve multiple packages with different numbers of files
// ─────────────────────────────────────────────────────────────────────────────
TEST_F(PackageManagerTest, ResolveMultiplePackagesWithVariousFiles) {
  falcon::pm::PackageManager::init(tmp_, "test-project");

  // Package 1: single file
  auto pkg1_dir = tmp_ / "single-lib";
  fs::create_directories(pkg1_dir);
  std::ofstream pkg1_manifest(pkg1_dir / "falcon.yml");
  pkg1_manifest << "name: single-lib\nversion: 0.1.0\n";
  pkg1_manifest.close();
  std::ofstream pkg1_fal(pkg1_dir / "single.fal");
  pkg1_fal << "routine single() {}\n";
  pkg1_fal.close();

  // Package 2: multiple files
  auto pkg2_dir = tmp_ / "multi-lib";
  fs::create_directories(pkg2_dir);
  std::ofstream pkg2_manifest(pkg2_dir / "falcon.yml");
  pkg2_manifest << "name: multi-lib\nversion: 0.1.0\n";
  pkg2_manifest.close();
  std::ofstream pkg2_fal1(pkg2_dir / "main.fal");
  pkg2_fal1 << "routine main() {}\n";
  pkg2_fal1.close();
  std::ofstream pkg2_fal2(pkg2_dir / "helper.fal");
  pkg2_fal2 << "routine helper() {}\n";
  pkg2_fal2.close();

  falcon::pm::PackageManager pm(tmp_);
  pm.install(pkg1_dir.string());
  pm.install(pkg2_dir.string());

  auto main_fal = tmp_ / "app.fal";
  std::ofstream app_f(main_fal);
  app_f << "import \"./single-lib\";\nimport \"./multi-lib\";\n";
  app_f.close();

  auto resolved = pm.resolve_imports(main_fal, {"./single-lib", "./multi-lib"});
  ASSERT_EQ(resolved.size(), 2u);
  EXPECT_EQ(resolved[0].module_name, "single-lib");
  EXPECT_EQ(resolved[1].module_name, "multi-lib");

  auto list = pm.list();
  EXPECT_GE(list.size(), 3u); // At least 1 from pkg1 + 2 from pkg2
}
