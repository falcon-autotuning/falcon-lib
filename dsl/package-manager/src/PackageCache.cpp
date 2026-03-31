#include "falcon-pm/PackageCache.hpp"
#include <fstream>
#include <openssl/evp.h>
#include <sstream>
#include <stdexcept>

namespace falcon::pm {

PackageCache::PackageCache(std::filesystem::path cache_dir)
    : cache_dir_(std::move(cache_dir)) {
  std::filesystem::create_directories(cache_dir_);
}

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

void PackageCache::clear() {
  std::filesystem::remove_all(cache_dir_);
  std::filesystem::create_directories(cache_dir_);
}

} // namespace falcon::pm
