#pragma once
#include "falcon-pm/PackageCache.hpp"
#include "falcon-pm/PackageManifest.hpp"
#include "falcon-pm/PackageResolver.hpp"
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace falcon::pm {

/**
 * @brief Installed-package descriptor (returned by list()).
 */
struct InstalledPackage {
  std::string name;
  std::string version;
  std::string github;
  std::filesystem::path cached_path;
};

/**
 * @brief Top-level API for the Falcon package manager.
 *
 * Used by:
 *  - AutotunerEngine (at load time, to resolve imports in .fal files)
 *  - falcon-pm CLI (for user-facing commands)
 *
 * Project root is determined by walking up from the given start path looking
 * for `falcon.yml`.  If no `falcon.yml` exists, operations that need it
 * (install, list) will fail; local-relative imports still work via the cache.
 */
class PackageManager {
public:
  /**
   * @brief Construct from any path inside the project (file or directory).
   *        Searches upward for falcon.yml to determine the project root.
   *        If no falcon.yml is found, project_root_ is set to start's
   *        directory — still allowing relative-path imports to function.
   */
  explicit PackageManager(const std::filesystem::path &start);

  // ── Init
  // ─��──────────────────────────────────────────────────────────────────
  /**
   * @brief Create a new `falcon.yml` and `.falcon/cache/` in `dir`.
   *        Fails if a `falcon.yml` already exists there.
   */
  static void init(const std::filesystem::path &dir,
                   const std::string &package_name);

  // ── Import resolution (used by AutotunerEngine) ──────────────────────────
  /**
   * @brief Resolve all import paths declared in `fal_file`, cache them,
   *        and return the list of resolved imports.
   *
   * @param fal_file  Absolute path to the .fal file being loaded.
   * @param imports   The `program->imports` vector from the parsed AST.
   */
  std::vector<PackageResolver::ResolvedImport>
  resolve_imports(const std::filesystem::path &fal_file,
                  const std::vector<std::string> &imports);
  /**
   * @brief Returns the path to falcon.yml or falcon.yaml if the directory is a
   * valid Falcon package (contains at least one .fal file).
   * @param dir Directory to check.
   * @return std::optional<std::filesystem::path> Path to manifest if valid,
   * else std::nullopt.
   */
  std::optional<std::filesystem::path>
  find_package_manifest(const std::filesystem::path &dir);

  // ── CLI-facing API ─────────────────────────────────────────────────────────
  /**
   * @brief List all packages currently in the cache index.
   */
  [[nodiscard]] std::vector<InstalledPackage> list() const;

  /**
   * @brief Install a package from a GitHub URL or local path, add it to
   *        falcon.yml dependencies, and cache the source.
   *
   * @param source   GitHub "owner/repo" or local filesystem path.
   * @param version  Version constraint (e.g. "^1.0.0") — used in falcon.yml.
   */
  void install(const std::string &source, const std::string &version = "*");

  /**
   * @brief Remove a package from the cache and from falcon.yml dependencies.
   */
  void remove(const std::string &package_name);

  /**
   * @brief Compiled and builds FFI bindings and hashes them for release
   *
   * @param dir Directory containing FFI code that needs to be compiled
   * @param extra_flags Additional flags indicating libraries and headers
   */
  void build(const std::filesystem::path &dir,
             const std::string &extra_flags = "");

  // ── Accessors ──────────────────────────────────────────────────────────────
  [[nodiscard]] const std::filesystem::path &project_root() const {
    return project_root_;
  }
  [[nodiscard]] PackageCache &cache() { return *cache_; }
  [[nodiscard]] PackageManifest &manifest() { return manifest_; }

private:
  std::filesystem::path project_root_;
  PackageManifest manifest_;
  std::unique_ptr<PackageCache> cache_;
  std::unique_ptr<PackageResolver> resolver_;
};

} // namespace falcon::pm
