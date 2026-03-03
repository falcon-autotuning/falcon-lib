#include "falcon-pm/PackageResolver.hpp"
#include "falcon-pm/PackageCache.hpp"
#include <stdexcept>

namespace falcon::pm {

PackageResolver::PackageResolver(std::filesystem::path project_root,
                                 PackageCache &cache)
    : project_root_(std::move(project_root)), cache_(cache) {}

PackageResolver::ResolvedImport
PackageResolver::resolve(const std::string &import_path,
                         const std::filesystem::path &importing_file) {
  auto base_dir = importing_file.parent_path();

  if (import_path.starts_with("./") || import_path.starts_with("../")) {
    return resolve_local(std::filesystem::path(import_path), base_dir);
  }
  if (import_path.starts_with("github.com/")) {
    return resolve_github(import_path);
  }
  // Bare filename — try relative to the importing file's directory first,
  // then relative to the project root.
  auto local_candidate = base_dir / import_path;
  if (std::filesystem::exists(local_candidate)) {
    return resolve_local(std::filesystem::path("./" + import_path), base_dir);
  }
  auto root_candidate = project_root_ / import_path;
  if (std::filesystem::exists(root_candidate)) {
    return resolve_local(import_path, project_root_);
  }
  throw std::runtime_error(
      "Cannot resolve import '" + import_path + "': not found relative to '" +
      base_dir.string() + "' or project root '" + project_root_.string() + "'");
}

std::vector<PackageResolver::ResolvedImport>
PackageResolver::resolve_all(const std::vector<std::string> &import_paths,
                             const std::filesystem::path &importing_file) {
  std::vector<ResolvedImport> results;
  results.reserve(import_paths.size());
  for (const auto &p : import_paths) {
    results.push_back(resolve(p, importing_file));
  }
  return results;
}

PackageResolver::ResolvedImport
PackageResolver::resolve_local(const std::filesystem::path &raw,
                               const std::filesystem::path &base_dir) {
  auto abs = std::filesystem::weakly_canonical(base_dir / raw);
  if (!std::filesystem::exists(abs)) {
    throw std::runtime_error("Import not found: " + abs.string());
  }
  // Cache it (store returns the cached path)
  auto cached = cache_.store(abs);
  auto sha = PackageCache::sha256_file(abs);
  auto module_name = abs.stem().string();

  return ResolvedImport{abs, cached, module_name, sha};
}

PackageResolver::ResolvedImport
PackageResolver::resolve_github(const std::string &import_path) {
  // TODO: implement HTTP download from GitHub raw content API.
  // For now, check vendor directory: <project_root>/vendor/<import_path>
  auto vendor_path = project_root_ / "vendor" / import_path;
  if (std::filesystem::exists(vendor_path)) {
    return resolve_local(vendor_path, project_root_);
  }
  throw std::runtime_error(
      "GitHub import not yet supported (no vendor copy found): " + import_path);
}

} // namespace falcon::pm
