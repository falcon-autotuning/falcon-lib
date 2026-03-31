#pragma once
#include <filesystem>
#include <map>
#include <optional>
#include <string>

namespace falcon::pm {

/**
 * @brief File cache for resolved .fal imports and released packages.
 *
 * Layout (inside `<project_root>/.falcon/cache/`):
 *
 *   .falcon/cache/
 *     <package1>/         ← each package is stored in its own directory
 *       ...               ← released package contents (e.g., .fal files,
 * manifest, etc.)
 *
 *     <package2>/
 *       ...
 *
 * Each package directory contains a released package snapshot.
 * The cache directory is created on first use.
 */
class PackageCache {
public:
  /**
   * @param cache_dir  Path to `.falcon/cache/` (created on first use).
   */
  explicit PackageCache(std::filesystem::path cache_dir);

  /**
   * @brief Compute SHA-256 of a file's contents.
   */
  static std::string sha256_file(const std::filesystem::path &path);

  /**
   * @brief Compute SHA-256 of an in-memory string.
   */
  static std::string sha256_string(const std::string &data);

  /**
   * @brief Remove all cache entries.
   */
  void clear();

  [[nodiscard]] const std::filesystem::path &cache_dir() const {
    return cache_dir_;
  }

private:
  std::filesystem::path cache_dir_;
};

} // namespace falcon::pm
