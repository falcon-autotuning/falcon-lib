#pragma once
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace falcon::pm {

/**
 * @brief Metadata about a package's structure and FFI dependencies.
 *
 * This is optional and primarily useful for documenting local packages.
 * For GitHub packages, structure is auto-detected.
 */
struct PackageInfo {
  // Root directory of the package (optional; can be inferred)
  std::optional<std::string> package_root;

  // Main .fal file (if not obvious from package structure)
  std::optional<std::string> main_file;

  // Map of FFI wrapper files (.so) to their hash that are part of this package
  std::map<std::string, std::string> ffi_wrappers;
};

/**
 * @brief Represents one entry in the `dependencies` list of falcon.yml.
 *
 * A dependency can be resolved from either:
 *  - A local path (local_path field)
 *  - A GitHub URL (github field, in format "owner/repo" or
 *    "github.com/owner/repo/path/file.fal")
 */
struct Dependency {
  std::string name;    // Package name (must match the dep's falcon.yml `name`)
  std::string version; // SemVer constraint, e.g. "^1.0.0"
  std::optional<std::string>
      github; // "owner/repo" or "github.com/owner/repo/path/file.fal"
  std::optional<std::string> local_path; // Relative path for local-only deps
  std::optional<PackageInfo>
      package_info; // Optional metadata about the package
};

/**
 * @brief In-memory representation of a `falcon.yml` project manifest.
 *
 * falcon.yml lives at the project root (the directory that `falcon-pm init`
 * was run in).  It serves the same role as Go's `go.mod`.
 */
struct PackageManifest {
  std::string name;       // Package / module name
  std::string version;    // SemVer, e.g. "1.0.0"
  std::string maintainer; // Free-form author/maintainer string
  std::string github;     // "owner/repo" of this package's canonical location
  std::string license;    // License for the package
  std::map<std::string, std::string>
      ffi; // binary -> hash mapping for all necessary cpp dependencies
  std::vector<Dependency> dependencies;
  // All of the dependant falcon packages this one uses

  /**
   * @brief Load a manifest from a `falcon.yml` file.
   * @throws std::runtime_error on parse failure.
   */
  static PackageManifest load(const std::filesystem::path &path);

  /**
   * @brief Write the manifest back to a `falcon.yml` file.
   */
  void save(const std::filesystem::path &path) const;

  /**
   * @brief Generate a minimal manifest skeleton (used by `falcon-pm init`).
   */
  static PackageManifest make_empty(const std::string &name);

  /**
   * @brief Search upward from `start` for the nearest `falcon.yml`.
   * @return The directory containing the found `falcon.yml`, or nullopt.
   */
  static std::optional<std::filesystem::path>
  find_root(const std::filesystem::path &start);
};

} // namespace falcon::pm
