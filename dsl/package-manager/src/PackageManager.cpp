#include "falcon-pm/PackageManager.hpp"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace falcon::pm {

PackageManager::PackageManager(const std::filesystem::path &start) {
  auto root_opt = PackageManifest::find_root(start);
  if (root_opt) {
    project_root_ = *root_opt;
    manifest_ = PackageManifest::load(project_root_ / "falcon.yml");
  } else {
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
    throw std::runtime_error("falcon.yml already exists in: " + dir.string());
  }
  std::filesystem::create_directories(dir / ".falcon" / "cache");
  auto m = PackageManifest::make_empty(package_name);
  m.save(manifest_path);
}

void PackageManager::build(const std::filesystem::path &dir,
                           const std::string &extra_flags) {
  auto manifest_path = dir / "falcon.yml";
  if (!std::filesystem::exists(manifest_path)) {
    throw std::runtime_error("Cannot build: no falcon.yml in " + dir.string());
  }

  auto m = PackageManifest::load(manifest_path);
  bool updated = false;

  for (auto &[so_name, hash] : m.ffi) {
    std::filesystem::path so_path = dir / so_name;
    std::filesystem::path cpp_path = so_path;
    cpp_path.replace_extension(".cpp");

    if (!std::filesystem::exists(cpp_path)) {
      throw std::runtime_error("Source file not found for FFI compilation: " +
                               cpp_path.string());
    }

    std::string cmd = "clang++ -std=c++17 -fPIC -shared -O2 -o \"" +
                      so_path.string() + "\" \"" + cpp_path.string() + "\"" +
                      extra_flags;
    std::cout << "Compiling " << cpp_path.filename().string() << " -> "
              << so_name << "...\n";

    int ret = system(cmd.c_str());
    if (ret != 0) {
      throw std::runtime_error("Compilation failed for " + cpp_path.string());
    }

    std::string new_hash = "sha256:" + PackageCache::sha256_file(so_path);
    if (hash != new_hash) {
      m.ffi[so_name] = new_hash;
      updated = true;
    }
  }

  if (updated) {
    m.save(manifest_path);
    std::cout << "Updated falcon.yml with new security hashes.\n";
  }
}

std::optional<std::filesystem::path>
PackageManager::find_package_manifest(const std::filesystem::path &dir) {
  if (!std::filesystem::is_directory(dir))
    return std::nullopt;
  if (std::filesystem::exists(dir / "falcon.yml"))
    return dir / "falcon.yml";
  if (std::filesystem::exists(dir / "falcon.yaml"))
    return dir / "falcon.yaml";
  return std::nullopt;
}

std::vector<PackageResolver::ResolvedImport>
PackageManager::resolve_imports(const std::filesystem::path &fal_file,
                                const std::vector<std::string> &imports) {
  return resolver_->resolve_all(imports, fal_file);
}

std::vector<InstalledPackage> PackageManager::list() const {
  std::vector<InstalledPackage> result;

  for (const auto &d : manifest_.dependencies) {
    InstalledPackage pkg;
    pkg.name = d.name;
    pkg.version = d.version;

    if (d.github) {
      pkg.github = *d.github;
      // Extract the repo name for the cache path
      auto slash_pos = d.github->find('/');
      std::string repo = (slash_pos != std::string::npos)
                             ? d.github->substr(slash_pos + 1)
                             : *d.github;
      pkg.cached_path = project_root_ / ".falcon" / "cache" / repo;
    } else if (d.local_path) {
      pkg.cached_path = *d.local_path;
    }

    result.push_back(std::move(pkg));
  }
  return result;
}

void PackageManager::install(const std::string &source,
                             const std::string &version) {
  bool is_github = source.starts_with("github.com/") ||
                   source.starts_with("https://github.com/");
  std::string normalized_source = source;
  if (normalized_source.starts_with("https://github.com/"))
    normalized_source = normalized_source.substr(8);

  if (is_github) {
    auto resolved =
        resolver_->resolve(normalized_source, project_root_ / "dummy.fal");
    std::string dep_name = resolved.module_name;

    for (const auto &d : manifest_.dependencies) {
      if (d.name == dep_name)
        return; // Already installed
    }

    Dependency d;
    d.name = dep_name;
    d.version = version;
    d.github = normalized_source;
    manifest_.dependencies.push_back(std::move(d));
    manifest_.save(project_root_ / "falcon.yml");
    std::cout << "Installed: " << normalized_source << "\n";

  } else {
    std::filesystem::path src_path(source);
    if (!PackageResolver::is_package(src_path)) {
      throw std::runtime_error(
          "install: source is not a Falcon package (missing falcon.yml): " +
          source);
    }

    std::string dep_name = src_path.stem().string();
    for (const auto &d : manifest_.dependencies) {
      if (d.name == dep_name)
        return;
    }

    Dependency d;
    d.name = dep_name;
    d.version = version;
    d.local_path = std::filesystem::weakly_canonical(src_path).string();
    manifest_.dependencies.push_back(std::move(d));
    manifest_.save(project_root_ / "falcon.yml");
    std::cout << "Installed: " << source << "\n";
  }
}

void PackageManager::remove(const std::string &package_name) {
  auto &deps = manifest_.dependencies;
  auto it = std::find_if(deps.begin(), deps.end(), [&](const Dependency &d) {
    return d.name == package_name;
  });

  if (it != deps.end()) {
    deps.erase(it);
    manifest_.save(project_root_ / "falcon.yml");
  } else {
    throw std::runtime_error("remove: package '" + package_name +
                             "' not found");
  }
}

} // namespace falcon::pm
