#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace falcon::pm {

class PackageCache;

/**
 * @brief Resolves an import path string into an absolute filesystem path.
 *
 * Supports:
 *  - Relative local paths:   "./Quantity.fal"  "./subdir/Adder.fal"
 *  - Future GitHub imports:  "github.com/owner/repo/path/file.fal"
 *                            (stubs out the download and returns a path in
 *                             the cache/vendor directory; actual HTTP fetch
 *                             is left as a TODO)
 *
 * Resolution order:
 *  1. If the path starts with "./" or "../" → resolve relative to the
 *     directory of the importing file.
 *  2. If the path starts with "github.com/" → look in the vendor directory
 *     or download (TODO).
 *  3. Otherwise → error.
 *
 * Module name derivation:
 *  The module name for a resolved file is the stem of its filename,
 *  case-preserved.  e.g. "./Quantity.fal" → module name "Quantity".
 */
class PackageResolver {
public:
  /**
   * @param project_root  Directory containing falcon.yml (used for vendor
   * lookup).
   * @param cache         SHA-256 cache (to validate/store resolved files).
   */
  PackageResolver(std::filesystem::path project_root, PackageCache &cache);

  struct ResolvedImport {
    std::filesystem::path absolute_path; ///< Absolute path to the .fal file
    std::filesystem::path cached_path;   ///< Path inside .falcon/cache/
    std::string module_name;             ///< e.g. "Quantity" from Quantity.fal
    std::string sha256;                  ///< Current SHA-256 of the source
  };

  /**
   * @brief Resolve one import path string.
   *
   * @param import_path  The raw string from `import "..."` in the .fal source.
   * @param importing_file  Absolute path of the file containing the import.
   * @throws std::runtime_error if the path cannot be resolved.
   */
  ResolvedImport resolve(const std::string &import_path,
                         const std::filesystem::path &importing_file);

  /**
   * @brief Resolve all imports declared in a .fal source's import list.
   */
  std::vector<ResolvedImport>
  resolve_all(const std::vector<std::string> &import_paths,
              const std::filesystem::path &importing_file);

private:
  ResolvedImport resolve_local(const std::filesystem::path &raw,
                               const std::filesystem::path &base_dir);
  ResolvedImport resolve_github(const std::string &import_path);

  std::filesystem::path project_root_;
  PackageCache &cache_;
};

} // namespace falcon::pm
