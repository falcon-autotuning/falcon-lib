#include "falcon-pm/PackageResolver.hpp"
#include "falcon-pm/PackageCache.hpp"
#include "falcon-pm/PackageManifest.hpp"
#include <algorithm>
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <yaml-cpp/yaml.h>

namespace falcon::pm {

namespace {
struct DownloadBuffer {
  std::string data;
  size_t write_callback(void *contents, size_t size, size_t nmemb) {
    size_t realsize = size * nmemb;
    data.append(static_cast<char *>(contents), realsize);
    return realsize;
  }
};

static size_t curl_write_callback(void *contents, size_t size, size_t nmemb,
                                  void *userp) {
  return static_cast<DownloadBuffer *>(userp)->write_callback(contents, size,
                                                              nmemb);
}
} // namespace

std::string PackageResolver::http_get(const std::string &url) {
  CURL *curl = curl_easy_init();
  if (!curl) {
    throw std::runtime_error("Failed to initialize CURL");
  }

  DownloadBuffer buffer;
  CURLcode res;

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
  curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

  res = curl_easy_perform(curl);
  std::string result = buffer.data;
  curl_easy_cleanup(curl);

  if (res != CURLE_OK) {
    throw std::runtime_error("HTTP request failed (" + url +
                             "): " + curl_easy_strerror(res));
  }

  return result;
}

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

  if (parts.size() < 3) {
    throw std::runtime_error("GitHub URL must have at least owner/repo/path: " +
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
  // The path given by the user IS the package directory
  // Examples:
  //  - "libs/collections/array"           (package directory)
  //  - "libs/collections/array/array.fal" (specific file in package)

  std::string package_root = url.path;
  std::string subpath = "";

  // If path ends with .fal, extract the filename as subpath
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
  // Simple approach: clone the repo to a cache directory
  auto cache_base = std::filesystem::temp_directory_path() / ".falcon-packages";
  std::filesystem::create_directories(cache_base);

  auto repo_cache_dir = cache_base / (owner + "_" + repo);

  // If already cloned, skip the clone step
  if (!std::filesystem::exists(repo_cache_dir)) {
    std::string clone_url = "https://github.com/" + owner + "/" + repo + ".git";
    std::string clone_cmd = "git clone --depth 1 --branch " + branch + " \"" +
                            clone_url + "\" \"" + repo_cache_dir.string() +
                            "\"";

    int ret = system(clone_cmd.c_str());
    if (ret != 0) {
      throw std::runtime_error(
          "Failed to clone repository: " + clone_url +
          "\n(Make sure git is installed and the URL is correct)");
    }
  }

  // Navigate to the package directory within the cloned repo
  auto final_path = repo_cache_dir / package_dir;

  if (!std::filesystem::exists(final_path)) {
    throw std::runtime_error(
        "Package directory not found in repository: " + package_dir +
        "\n(Looked in: " + final_path.string() + ")");
  }

  // Verify it's a Falcon package (has falcon.yml)
  if (!std::filesystem::exists(final_path / "falcon.yml")) {
    throw std::runtime_error(
        "No falcon.yml found in package directory: " + package_dir +
        "\n(Looked in: " + final_path.string() + ")");
  }

  return final_path;
}

std::string PackageResolver::get_package_main_file(
    const std::filesystem::path &package_root) {
  // Read falcon.yml to determine the main file
  auto manifest_file = package_root / "falcon.yml";

  if (std::filesystem::exists(manifest_file)) {
    try {
      auto manifest = PackageManifest::load(manifest_file);
      // The "main" file should be the first .fal file in the package
      // For now, use the package name with .fal extension
      std::string pkg_name = package_root.stem().string();
      return pkg_name + ".fal";
    } catch (const YAML::Exception &e) {
      // Fallback below
    }
  }

  // Fallback: look for .fal file with same name as directory
  std::string dir_name = package_root.stem().string();
  auto default_main = package_root / (dir_name + ".fal");

  if (std::filesystem::exists(default_main)) {
    return dir_name + ".fal";
  }

  // Fallback: any .fal file in the root
  for (const auto &entry : std::filesystem::directory_iterator(package_root)) {
    if (entry.path().extension() == ".fal") {
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

  // Try relative to importing file
  auto local_candidate = base_dir / import_path;
  if (std::filesystem::exists(local_candidate)) {
    return resolve_local(std::filesystem::path("./" + import_path), base_dir);
  }

  // Try relative to project root
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

  // Check if it's a package directory
  if (is_package(abs)) {
    // Load manifest to get FFI wrappers
    auto manifest = PackageManifest::load(abs / "falcon.yml");

    // Cache ALL .fal files in the package
    std::vector<std::filesystem::path> fal_files;
    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(abs)) {
      if (entry.path().extension() == ".fal") {
        cache_.store(entry.path());
        fal_files.push_back(entry.path());
      }
    }

    // Cache all FFI wrapper files
    for (const auto &wrapper_rel : manifest.ffi) {
      auto wrapper_path = abs / wrapper_rel;
      if (std::filesystem::exists(wrapper_path)) {
        cache_.store(wrapper_path);
      }
      // Note: missing wrappers don't cause failure
    }

    if (fal_files.empty()) {
      throw std::runtime_error("No .fal files found in package: " +
                               abs.string());
    }

    auto main_path = fal_files[0];
    auto sha = PackageCache::sha256_file(main_path);
    auto module_name = abs.stem().string();

    return ResolvedImport{
        main_path, cache_.lookup(main_path).value_or(main_path),
        abs,       module_name,
        sha,       true};
  }

  // Regular .fal file
  if (!std::filesystem::exists(abs)) {
    throw std::runtime_error("Import not found: " + abs.string());
  }

  auto cached = cache_.store(abs);
  auto sha = PackageCache::sha256_file(abs);
  auto module_name = abs.stem().string();
  auto package_root = abs.parent_path();

  return ResolvedImport{abs, cached, package_root, module_name, sha, false};
}

PackageResolver::ResolvedImport
PackageResolver::resolve_github_package(const std::string &import_path) {
  auto github_url = parse_github_url(import_path);
  auto [package_dir, subpath] = find_package_root_in_path(github_url);
  auto pkg_root = download_github_package(github_url.owner, github_url.repo,
                                          github_url.branch, package_dir);

  // Load manifest to get FFI wrappers
  auto manifest = PackageManifest::load(pkg_root / "falcon.yml");

  // Cache ALL .fal files in the package
  std::vector<std::filesystem::path> fal_files;
  for (const auto &entry :
       std::filesystem::recursive_directory_iterator(pkg_root)) {
    if (entry.path().extension() == ".fal") {
      cache_.store(entry.path());
      fal_files.push_back(entry.path());
    }
  }

  // Cache all FFI wrapper files
  for (const auto &wrapper_rel : manifest.ffi) {
    auto wrapper_path = pkg_root / wrapper_rel;
    if (std::filesystem::exists(wrapper_path)) {
      cache_.store(wrapper_path);
    }
    // Note: missing wrappers don't cause failure
  }

  if (fal_files.empty()) {
    throw std::runtime_error("No .fal files found in downloaded package: " +
                             pkg_root.string());
  }

  auto file_abs_path = fal_files[0];
  auto sha = PackageCache::sha256_file(file_abs_path);

  std::string module_name;
  if (subpath.empty()) {
    module_name = pkg_root.stem().string();
  } else {
    module_name = file_abs_path.stem().string();
  }

  return ResolvedImport{
      file_abs_path, cache_.lookup(file_abs_path).value_or(file_abs_path),
      pkg_root,      module_name,
      sha,           true};
}

} // namespace falcon::pm
