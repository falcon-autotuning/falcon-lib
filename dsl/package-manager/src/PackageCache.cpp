#include "falcon-pm/PackageCache.hpp"
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <openssl/evp.h>
#include <sstream>
#include <stdexcept>

namespace falcon::pm {

PackageCache::PackageCache(std::filesystem::path cache_dir)
    : cache_dir_(std::move(cache_dir)), index_path_(cache_dir_ / "index.json") {
  std::filesystem::create_directories(cache_dir_);
  load_index();
}

// ─── SHA-256 helpers ─────────────────────────────────────────────────────────

static std::string bytes_to_hex(const unsigned char *buf, size_t len) {
  std::ostringstream oss;
  oss << std::hex;
  for (size_t i = 0; i < len; ++i) {
    oss.width(2);
    oss.fill('0');
    oss << static_cast<unsigned>(buf[i]);
  }
  return oss.str();
}

std::string PackageCache::sha256_string(const std::string &data) {
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  if (!ctx)
    throw std::runtime_error("EVP_MD_CTX_new failed");

  unsigned char hash[EVP_MAX_MD_SIZE];
  unsigned int hash_len = 0;

  if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1 ||
      EVP_DigestUpdate(ctx, data.data(), data.size()) != 1 ||
      EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1) {
    EVP_MD_CTX_free(ctx);
    throw std::runtime_error("SHA-256 digest failed");
  }
  EVP_MD_CTX_free(ctx);
  return bytes_to_hex(hash, hash_len);
}

std::string PackageCache::sha256_file(const std::filesystem::path &path) {
  std::ifstream f(path, std::ios::binary);
  if (!f.is_open())
    throw std::runtime_error("Cannot open for hashing: " + path.string());
  std::ostringstream ss;
  ss << f.rdbuf();
  return sha256_string(ss.str());
}

// ─── Index persistence ───────────────────────────────────────────────────────

void PackageCache::load_index() {
  index_.clear();
  if (!std::filesystem::exists(index_path_))
    return;
  std::ifstream f(index_path_);
  if (!f.is_open())
    return;
  try {
    auto j = nlohmann::json::parse(f);
    for (auto it = j.begin(); it != j.end(); ++it) {
      CacheEntry e;
      e.sha256 = it.value().at("sha256").get<std::string>();
      e.cached_path = it.value().at("cached_path").get<std::string>();
      index_[it.key()] = std::move(e);
    }
  } catch (...) {
    // Corrupt index — start fresh
    index_.clear();
  }
}

void PackageCache::save_index() const {
  nlohmann::json j;
  for (const auto &[key, entry] : index_) {
    j[key] = {{"sha256", entry.sha256},
              {"cached_path", entry.cached_path.string()}};
  }
  std::ofstream f(index_path_);
  if (!f.is_open())
    throw std::runtime_error("Cannot write cache index: " +
                             index_path_.string());
  f << j.dump(2);
}

std::optional<PackageCache::CacheEntry>
PackageCache::find_entry(const std::string &key) const {
  auto it = index_.find(key);
  if (it == index_.end())
    return std::nullopt;
  return it->second;
}

// ─── Public API ──────────────────────────────────────────────────────────────

std::optional<std::filesystem::path>
PackageCache::lookup(const std::filesystem::path &source_path) const {
  auto key = std::filesystem::weakly_canonical(source_path).string();
  auto entry = find_entry(key);
  if (!entry)
    return std::nullopt;

  // Check the cached file still exists
  if (!std::filesystem::exists(entry->cached_path))
    return std::nullopt;

  // Validate: source file must match stored SHA-256
  try {
    auto current_sha = sha256_file(source_path);
    if (current_sha != entry->sha256)
      return std::nullopt; // stale
  } catch (...) {
    return std::nullopt;
  }
  return entry->cached_path;
}

std::filesystem::path
PackageCache::store(const std::filesystem::path &source_path) {
  auto key = std::filesystem::weakly_canonical(source_path).string();
  auto sha = sha256_file(source_path);
  auto dest = cache_dir_ / (sha + ".fal");

  // Copy source → cache (overwrite if already exists with same sha —
  // idempotent)
  std::filesystem::copy_file(source_path, dest,
                             std::filesystem::copy_options::overwrite_existing);

  index_[key] = CacheEntry{sha, dest};
  save_index();
  return dest;
}

void PackageCache::invalidate(const std::filesystem::path &source_path) {
  auto key = std::filesystem::weakly_canonical(source_path).string();
  auto it = index_.find(key);
  if (it == index_.end())
    return;

  // Remove the cached file if no other entry references it
  auto cached = it->second.cached_path;
  index_.erase(it);
  save_index();

  bool still_referenced = false;
  for (const auto &[k, e] : index_) {
    if (e.cached_path == cached) {
      still_referenced = true;
      break;
    }
  }
  if (!still_referenced && std::filesystem::exists(cached))
    std::filesystem::remove(cached);
}

void PackageCache::clear() {
  for (const auto &[key, entry] : index_) {
    if (std::filesystem::exists(entry.cached_path))
      std::filesystem::remove(entry.cached_path);
  }
  index_.clear();
  if (std::filesystem::exists(index_path_))
    std::filesystem::remove(index_path_);
}

} // namespace falcon::pm
