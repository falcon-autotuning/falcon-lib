#include "falcon-pm/PackageResolver.hpp"
#include "falcon-pm/PackageCache.hpp"
#include "falcon-pm/PackageManifest.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace falcon::pm {

PackageResolver::GitHubURL
PackageResolver::parse_github_url(const std::string &url_string) {
  if (!url_string.starts_with("github.com/")) {
    throw std::runtime_error("Invalid GitHub URL: " + url_string);
  }

  std::string url = url_string.substr(11);
  GitHubURL result;
  result.branch = "main";

  std::istringstream iss(url);
  std::string token;
  std::vector<std::string> parts;

  while (std::getline(iss, token, '/')) {
    if (!token.empty()) {
      parts.push_back(token);
    }
  }

  if (parts.size() < 2) {
    throw std::runtime_error("GitHub URL must have at least owner/repo: " +
                             url_string);
  }

  result.owner = parts[0];
  std::string repo_and_branch = parts[1];

  size_t at_pos = repo_and_branch.find('@');
  if (at_pos != std::string::npos) {
    result.repo = repo_and_branch.substr(0, at_pos);
    result.branch = repo_and_branch.substr(at_pos + 1);
  } else {
    result.repo = repo_and_branch;
  }

  for (size_t i = 2; i < parts.size(); ++i) {
    if (i > 2)
      result.path += "/";
    result.path += parts[i];
  }

  return result;
}

std::pair<std::string, std::string>
PackageResolver::find_package_root_in_path(const GitHubURL &url) {
  std::string package_root = url.path;
  std::string subpath = "";

  if (package_root.ends_with(".fal")) {
    size_t last_slash = package_root.find_last_of('/');
    if (last_slash != std::string::npos) {
      subpath = package_root.substr(last_slash + 1);
      package_root = package_root.substr(0, last_slash);
    }
  }

  return {package_root, subpath};
}

std::filesystem::path PackageResolver::download_github_package(
    const std::string &owner, const std::string &repo,
    const std::string &branch, const std::string &package_dir) {

  auto repo_cache_dir = cache_.cache_dir() / repo;

  if (!std::filesystem::exists(repo_cache_dir)) {
    std::filesystem::create_directories(repo_cache_dir);

    std::filesystem::path temp_tar =
        cache_.cache_dir() / (repo + "_temp.tar.gz");

    // Try Release Tarball First (safely without pipe errors)
    std::string tar_url = "https://github.com/" + owner + "/" + repo +
                          "/releases/latest/download/" + repo + ".tar.gz";
    std::string dl_cmd =
        "curl -sL --fail -o \"" + temp_tar.string() + "\" \"" + tar_url + "\"";

    int ret = system(dl_cmd.c_str());
    if (ret == 0) {
      std::string extract_cmd = "tar -xzf \"" + temp_tar.string() + "\" -C \"" +
                                repo_cache_dir.string() + "\"";
      system(extract_cmd.c_str());
    } else {
      // Fallback to Source Code Archive
      tar_url = "https://github.com/" + owner + "/" + repo +
                "/archive/refs/heads/" + branch + ".tar.gz";
      dl_cmd = "curl -sL --fail -o \"" + temp_tar.string() + "\" \"" + tar_url +
               "\"";
      ret = system(dl_cmd.c_str());

      if (ret == 0) {
        std::string extract_cmd = "tar -xzf \"" + temp_tar.string() +
                                  "\" --strip-components=1 -C \"" +
                                  repo_cache_dir.string() + "\"";
        system(extract_cmd.c_str());
      } else {
        std::filesystem::remove_all(repo_cache_dir);
        if (std::filesystem::exists(temp_tar))
          std::filesystem::remove(temp_tar);
        throw std::runtime_error("Failed to download package from " + tar_url);
      }
    }
    if (std::filesystem::exists(temp_tar))
      std::filesystem::remove(temp_tar);
  }

  auto final_path = repo_cache_dir;
  if (!package_dir.empty() && package_dir != "/") {
    final_path = repo_cache_dir / package_dir;
  }

  if (!std::filesystem::exists(final_path / "falcon.yml")) {
    throw std::runtime_error(
        "No falcon.yml found in downloaded package directory: " +
        final_path.string());
  }

  auto manifest = PackageManifest::load(final_path / "falcon.yml");
  for (const auto &[so_name, expected_hash] : manifest.ffi) {
    if (expected_hash.starts_with("sha256:")) {
      auto so_path = final_path / so_name;
      if (!std::filesystem::exists(so_path)) {
        throw std::runtime_error("Missing expected FFI binary in package: " +
                                 so_name);
      }
      std::string actual_hash = "sha256:" + PackageCache::sha256_file(so_path);
      if (actual_hash != expected_hash) {
        throw std::runtime_error("Security Error! Hash mismatch for " +
                                 so_name + "\nExpected: " + expected_hash +
                                 "\nActual:   " + actual_hash);
      }
    }
  }

  return final_path;
}

std::string PackageResolver::get_package_main_file(
    const std::filesystem::path &package_root) {
  auto manifest_file = package_root / "falcon.yml";

  if (std::filesystem::exists(manifest_file)) {
    try {
      auto manifest = PackageManifest::load(manifest_file);
      std::string pkg_name = manifest.name;
      if (pkg_name.empty())
        pkg_name = package_root.stem().string();
      if (std::filesystem::exists(package_root / (pkg_name + ".fal"))) {
        return pkg_name + ".fal";
      }
    } catch (...) {
    }
  }

  std::string dir_name = package_root.stem().string();
  if (std::filesystem::exists(package_root / (dir_name + ".fal"))) {
    return dir_name + ".fal";
  }

  for (const auto &entry : std::filesystem::directory_iterator(package_root)) {
    if (entry.is_regular_file() && entry.path().extension() == ".fal") {
      return entry.path().filename().string();
    }
  }

  throw std::runtime_error("No main .fal file found in package: " +
                           package_root.string());
}

bool PackageResolver::is_package(const std::filesystem::path &path) {
  if (std::filesystem::is_directory(path)) {
    return std::filesystem::exists(path / "falcon.yml");
  }
  return false;
}

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
    return resolve_github_package(import_path);
  }

  auto local_candidate = base_dir / import_path;
  if (std::filesystem::exists(local_candidate)) {
    return resolve_local(std::filesystem::path("./" + import_path), base_dir);
  }

  auto root_candidate = project_root_ / import_path;
  if (std::filesystem::exists(root_candidate)) {
    return resolve_local(import_path, project_root_);
  }

  throw std::runtime_error("Cannot resolve import '" + import_path + "'");
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

  if (is_package(abs)) {
    resolve_package_dependencies(abs);

    std::filesystem::path main_path;
    try {
      main_path = abs / get_package_main_file(abs);
    } catch (...) {
      throw std::runtime_error("No .fal files found in package: " +
                               abs.string());
    }

    return ResolvedImport{main_path,           main_path, abs,
                          abs.stem().string(), "",        true};
  }

  if (!std::filesystem::exists(abs)) {
    throw std::runtime_error("Import not found: " + abs.string());
  }

  // RE-ADDED: Compute the SHA256 for local files so the test passes and caching
  // can utilize it
  std::string sha = PackageCache::sha256_file(abs);

  return ResolvedImport{abs, abs,  abs.parent_path(), abs.stem().string(),
                        sha, false};
}

PackageResolver::ResolvedImport
PackageResolver::resolve_github_package(const std::string &import_path) {
  auto github_url = parse_github_url(import_path);
  auto [package_dir, subpath] = find_package_root_in_path(github_url);
  auto pkg_root = download_github_package(github_url.owner, github_url.repo,
                                          github_url.branch, package_dir);

  resolve_package_dependencies(pkg_root);

  std::filesystem::path file_abs_path;
  if (subpath.empty()) {
    file_abs_path = pkg_root / get_package_main_file(pkg_root);
  } else {
    file_abs_path = pkg_root / subpath;
  }

  std::string module_name = subpath.empty() ? pkg_root.stem().string()
                                            : file_abs_path.stem().string();

  return ResolvedImport{file_abs_path, file_abs_path, pkg_root, module_name, "",
                        true};
}

void PackageResolver::resolve_package_dependencies(
    const std::filesystem::path &pkg_root) {
  auto manifest_path = pkg_root / "falcon.yml";
  if (!std::filesystem::exists(manifest_path))
    return;

  auto manifest = PackageManifest::load(manifest_path);
  for (const auto &dep : manifest.dependencies) {
    try {
      std::string import_path;
      if (dep.github)
        import_path = *dep.github;
      else if (dep.local_path)
        import_path = *dep.local_path;
      else
        continue;

      resolve(import_path, pkg_root / "dummy.fal");
    } catch (...) {
    }
  }
}

} // namespace falcon::pm
