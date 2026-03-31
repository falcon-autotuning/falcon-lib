#pragma once
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace falcon::pm {

class PackageCache;

/**
 * @brief Resolves an import path string into an absolute filesystem path.
 *
 * Supports:
 *  - Relative local paths:   "./Quantity.fal"  "./subdir/Adder.fal"
 *  - Local packages:         "../shared/array" (directory with falcon.yml)
 *  - GitHub packages:        "github.com/owner/repo/libs/collections/array"
 *
 * A Falcon package is identified by a `falcon.yml` metadata file in its root
 * directory.
 */
class PackageResolver {
public:
  PackageResolver(std::filesystem::path project_root, PackageCache &cache);

  struct ResolvedImport {
    std::filesystem::path absolute_path; ///< Absolute path to the .fal file
    std::filesystem::path cached_path;   ///< Path inside .falcon/cache/
    std::filesystem::path
        package_root; ///< Root of the package (for FFI/relative resolution)
    std::string module_name; ///< e.g. "array" from package name
    std::string sha256;      ///< SHA-256 of the resolved file
    bool is_package;         ///< True if this is a package import
  };

  /**
   * @brief Resolve one import path string.
   *
   * @param import_path  The raw string from `import "..."` in the .fal source.
   * @param importing_file  Absolute path of the file containing the import.
   * @return Resolved import information
   * @throws std::runtime_error if the path cannot be resolved or downloaded.
   */
  ResolvedImport resolve(const std::string &import_path,
                         const std::filesystem::path &importing_file);

  /**
   * @brief Resolve all imports declared in a .fal source's import list.
   */
  std::vector<ResolvedImport>
  resolve_all(const std::vector<std::string> &import_paths,
              const std::filesystem::path &importing_file);

  /**
   * @brief Detect if a path is a Falcon package.
   *
   * A Falcon package must have a falcon.yml file at its root.
   *
   * @param path Local filesystem path or GitHub URL
   * @return True if the path is a Falcon package
   */
  static bool is_package(const std::filesystem::path &path);

private:
  /**
   * @brief Recursively resolve and cache all dependencies of a package.
   *
   * When a package is resolved, we need to also resolve and cache all of its
   * dependencies (and their dependencies, recursively) so that FFI wrappers
   * and .fal files from the entire dependency tree are available.
   */
  void resolve_package_dependencies(const std::filesystem::path &pkg_root);
  struct GitHubURL {
    std::string owner;
    std::string repo;
    std::string branch;
    std::string path;
  };

  ResolvedImport resolve_local(const std::filesystem::path &raw,
                               const std::filesystem::path &base_dir);

  ResolvedImport resolve_github_package(const std::string &import_path);

  static GitHubURL parse_github_url(const std::string &url_string);

  /**
   * @brief Find the falcon.yml file in the GitHub path hierarchy.
   *
   * Walks up the directory tree to find the falcon.yml that identifies the
   * package.
   *
   * @return Pair of (package_directory, remaining_subpath)
   */
  static std::pair<std::string, std::string>
  find_package_root_in_path(const GitHubURL &url);

  std::filesystem::path download_github_package(const std::string &owner,
                                                const std::string &repo,
                                                const std::string &branch,
                                                const std::string &package_dir);

  static std::string
  get_package_main_file(const std::filesystem::path &package_root);

  static std::string http_get(const std::string &url);

  std::filesystem::path project_root_;
  PackageCache &cache_;
};

} // namespace falcon::pm
