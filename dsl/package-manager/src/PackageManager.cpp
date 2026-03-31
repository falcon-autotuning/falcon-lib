#include "falcon-pm/PackageManager.hpp"
#include <algorithm>
#include <stdexcept>

namespace falcon::pm {

PackageManager::PackageManager(const std::filesystem::path &start) {
  auto root_opt = PackageManifest::find_root(start);
  if (root_opt) {
    project_root_ = *root_opt;
    manifest_ = PackageManifest::load(project_root_ / "falcon.yml");
  } else {
    // No falcon.yml — work relative to start directory (allows bare import
    // tests)
    project_root_ =
        std::filesystem::is_directory(start) ? start : start.parent_path();
    manifest_ = PackageManifest::make_empty("(unnamed)");
  }

  auto cache_dir = project_root_ / ".falcon" / "cache";
  cache_ = std::make_unique<PackageCache>(cache_dir);
  resolver_ = std::make_unique<PackageResolver>(project_root_, *cache_);
}

void PackageManager::init(const std::filesystem::path &dir,
                          const std::string &package_name) {
  auto manifest_path = dir / "falcon.yml";
  if (std::filesystem::exists(manifest_path)) {
    throw std::runtime_error("falcon.yml already exists in: " + dir.string() +
                             "  (use `falcon-pm install` to add dependencies)");
  }
  std::filesystem::create_directories(dir / ".falcon" / "cache");
  auto m = PackageManifest::make_empty(package_name);
  m.save(manifest_path);
}

std::vector<PackageResolver::ResolvedImport>
PackageManager::resolve_imports(const std::filesystem::path &fal_file,
                                const std::vector<std::string> &imports) {
  return resolver_->resolve_all(imports, fal_file);
}

std::vector<InstalledPackage> PackageManager::list() const {
  // Walk the cache index and correlate with manifest dependencies
  std::vector<InstalledPackage> result;

  // Build a quick lookup from dep name → Dependency
  std::map<std::string, const Dependency *> dep_map;
  for (const auto &d : manifest_.dependencies) {
    dep_map[d.name] = &d;
  }

  // Enumerate cached entries (each sha.fal corresponds to one source file)
  auto cache_dir = project_root_ / ".falcon" / "cache";
  if (!std::filesystem::exists(cache_dir)) {
    return result;
  }

  for (const auto &entry : std::filesystem::directory_iterator(cache_dir)) {
    if (entry.path().extension() != ".fal") {
      continue;
    }
    InstalledPackage pkg;
    pkg.cached_path = entry.path();
    pkg.cached_sha256 = entry.path().stem().string();
    // Module name isn't directly recoverable from the sha filename alone;
    // we'd need the index.  For a richer list, the index could store the
    // module name too.  For now, use the sha as the identifier.
    pkg.name = pkg.cached_sha256.substr(0, 12) + "...";
    pkg.version = "cached";
    result.push_back(std::move(pkg));
  }
  return result;
}

void PackageManager::install(const std::string &source,
                             const std::string &version) {
  // For now: local path installation only.
  // GitHub installation is a TODO (see PackageResolver::resolve_github).
  std::filesystem::path src_path(source);
  if (!std::filesystem::exists(src_path)) {
    throw std::runtime_error("install: source not found: " + source);
  }

  // Recursively cache all .fal files under source path
  if (std::filesystem::is_directory(src_path)) {
    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(src_path)) {
      if (entry.path().extension() == ".fal") {
        cache_->store(entry.path());
      }
    }
  } else {
    cache_->store(src_path);
  }

  // Add to manifest if not already present
  std::string dep_name = src_path.stem().string();
  bool already = false;
  for (const auto &d : manifest_.dependencies) {
    if (d.name == dep_name) {
      already = true;
      break;
    }
  }

  if (!already) {
    Dependency d;
    d.name = dep_name;
    d.version = version;
    d.local_path = std::filesystem::weakly_canonical(src_path).string();
    manifest_.dependencies.push_back(std::move(d));

    auto manifest_path = project_root_ / "falcon.yml";
    if (std::filesystem::exists(manifest_path)) {
      manifest_.save(manifest_path);
    }
  }
}

void PackageManager::remove(const std::string &package_name) {
  // Remove from manifest
  auto &deps = manifest_.dependencies;
  auto it = std::find_if(deps.begin(), deps.end(), [&](const Dependency &d) {
    return d.name == package_name;
  });
  if (it != deps.end()) {
    // If it has a local_path, invalidate the cache entry
    if (it->local_path) {
      try {
        cache_->invalidate(std::filesystem::path(*it->local_path));
      } catch (...) {
      }
    }
    deps.erase(it);
    auto manifest_path = project_root_ / "falcon.yml";
    if (std::filesystem::exists(manifest_path))
      manifest_.save(manifest_path);
  } else {
    throw std::runtime_error("remove: package '" + package_name +
                             "' not found in manifest");
  }
}

} // namespace falcon::pm
