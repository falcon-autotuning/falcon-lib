#pragma once
#include <filesystem>
#include <map>
#include <optional>
#include <string>

namespace falcon::pm {

/**
 * @brief SHA-256-based file cache for resolved .fal imports.
 *
 * Layout (inside `<project_root>/.falcon/cache/`):
 *
 *   .falcon/cache/
 *     index.json                   ← maps "origin path" → {sha256, cached_path}
 *     <sha256hex>.fal              ← cached source copy
 *
 * A file is "fresh" if its on-disk SHA-256 matches the cached SHA-256.
 * If fresh, the cached copy is returned directly — no re-parse needed.
 * If stale (source changed), the cache entry is updated.
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
   * @brief Check whether `source_path` is cached and up-to-date.
   *
   * @return The path to the cached copy if valid, or nullopt if missing/stale.
   */
  std::optional<std::filesystem::path>
  lookup(const std::filesystem::path &source_path) const;

  /**
   * @brief Store (or refresh) a source file in the cache.
   *
   * @return Path to the cached copy.
   */
  std::filesystem::path store(const std::filesystem::path &source_path);

  /**
   * @brief Remove the cache entry for `source_path` (if any).
   */
  void invalidate(const std::filesystem::path &source_path);

  /**
   * @brief Remove all cache entries.
   */
  void clear();

  [[nodiscard]] const std::filesystem::path &cache_dir() const {
    return cache_dir_;
  }

private:
  struct CacheEntry {
    std::string sha256;
    std::filesystem::path cached_path;
  };

  void load_index();
  void save_index() const;
  std::optional<CacheEntry> find_entry(const std::string &key) const;

  std::filesystem::path cache_dir_;
  std::filesystem::path index_path_;

  // In-memory index: normalised source path string → CacheEntry
  mutable std::map<std::string, CacheEntry> index_;
};

} // namespace falcon::pm
