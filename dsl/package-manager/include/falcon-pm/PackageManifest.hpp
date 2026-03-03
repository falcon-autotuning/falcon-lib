#pragma once
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace falcon::pm {

/**
 * @brief Represents one entry in the `dependencies` list of falcon.yml.
 */
struct Dependency {
  std::string name;    // Package name (must match the dep's falcon.yml `name`)
  std::string version; // SemVer constraint, e.g. "^1.0.0"
  std::optional<std::string> github; // "owner/repo" — source for remote fetch
  std::optional<std::string> local_path; // Relative path for local-only deps
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
  std::vector<Dependency> dependencies;

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
