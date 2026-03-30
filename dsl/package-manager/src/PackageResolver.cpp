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
  auto cache_base = std::filesystem::temp_directory_path() / ".falcon-packages";
  std::filesystem::create_directories(cache_base);

  auto repo_cache_dir = cache_base / (owner + "_" + repo);

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

  auto final_path = repo_cache_dir / package_dir;

  if (!std::filesystem::exists(final_path)) {
    throw std::runtime_error(
        "Package directory not found in repository: " + package_dir +
        "\n(Looked in: " + final_path.string() + ")");
  }

  if (!std::filesystem::exists(final_path / "falcon.yml")) {
    throw std::runtime_error(
        "No falcon.yml found in package directory: " + package_dir +
        "\n(Looked in: " + final_path.string() + ")");
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
      if (pkg_name.empty()) {
        pkg_name = package_root.stem().string();
      }
      if (std::filesystem::exists(package_root / (pkg_name + ".fal"))) {
        return pkg_name + ".fal";
      }
    } catch (const YAML::Exception &e) {
      // Fallback below
    }
  }

  std::string dir_name = package_root.stem().string();
  auto default_main = package_root / (dir_name + ".fal");

  if (std::filesystem::exists(default_main)) {
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

  if (is_package(abs)) {
    auto manifest = PackageManifest::load(abs / "falcon.yml");

    std::vector<std::filesystem::path> fal_files;
    for (auto it = std::filesystem::recursive_directory_iterator(abs);
         it != std::filesystem::recursive_directory_iterator(); ++it) {
      if (it->is_directory() && it->path().filename() == ".falcon") {
        it.disable_recursion_pending();
        continue;
      }
      if (it->is_regular_file() && it->path().extension() == ".fal") {
        cache_.store(it->path());
        fal_files.push_back(it->path());
      }
    }

    for (const auto &wrapper_rel : manifest.ffi) {
      auto wrapper_path = abs / wrapper_rel;
      if (std::filesystem::exists(wrapper_path)) {
        auto cached_wrapper = cache_.cache_dir() / wrapper_rel;
        std::filesystem::create_directories(cached_wrapper.parent_path());
        std::filesystem::copy_file(
            wrapper_path, cached_wrapper,
            std::filesystem::copy_options::overwrite_existing);
      }
    }

    resolve_package_dependencies(abs);

    if (fal_files.empty()) {
      throw std::runtime_error("No .fal files found in package: " +
                               abs.string());
    }

    std::filesystem::path main_path;
    try {
      auto main_file_name = get_package_main_file(abs);
      main_path = abs / main_file_name;
    } catch (...) {
    }

    if (!std::filesystem::exists(main_path)) {
      main_path = fal_files[0];
    }

    auto sha = PackageCache::sha256_file(main_path);
    auto module_name = abs.stem().string();

    return ResolvedImport{main_path,
                          cache_.lookup(main_path).value_or(main_path),
                          cache_.cache_dir(),
                          module_name,
                          sha,
                          true};
  }

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

  auto manifest = PackageManifest::load(pkg_root / "falcon.yml");

  std::vector<std::filesystem::path> fal_files;
  for (auto it = std::filesystem::recursive_directory_iterator(pkg_root);
       it != std::filesystem::recursive_directory_iterator(); ++it) {
    if (it->is_directory() && it->path().filename() == ".falcon") {
      it.disable_recursion_pending();
      continue;
    }
    if (it->is_regular_file() && it->path().extension() == ".fal") {
      cache_.store(it->path());
      fal_files.push_back(it->path());
    }
  }

  for (const auto &wrapper_rel : manifest.ffi) {
    auto wrapper_path = pkg_root / wrapper_rel;
    if (std::filesystem::exists(wrapper_path)) {
      auto cached_wrapper = cache_.cache_dir() / wrapper_rel;
      std::filesystem::create_directories(cached_wrapper.parent_path());
      std::filesystem::copy_file(
          wrapper_path, cached_wrapper,
          std::filesystem::copy_options::overwrite_existing);
    }
  }

  resolve_package_dependencies(pkg_root);

  if (fal_files.empty()) {
    throw std::runtime_error("No .fal files found in downloaded package: " +
                             pkg_root.string());
  }

  std::filesystem::path file_abs_path;
  if (subpath.empty()) {
    try {
      auto main_file_name = get_package_main_file(pkg_root);
      file_abs_path = pkg_root / main_file_name;
    } catch (...) {
    }

    if (!std::filesystem::exists(file_abs_path)) {
      file_abs_path = fal_files[0];
    }
  } else {
    file_abs_path = pkg_root / subpath;
    if (!std::filesystem::exists(file_abs_path)) {
      file_abs_path = fal_files[0];
    }
  }

  auto sha = PackageCache::sha256_file(file_abs_path);

  std::string module_name;
  if (subpath.empty()) {
    module_name = pkg_root.stem().string();
  } else {
    module_name = file_abs_path.stem().string();
  }

  return ResolvedImport{file_abs_path,
                        cache_.lookup(file_abs_path).value_or(file_abs_path),
                        cache_.cache_dir(),
                        module_name,
                        sha,
                        true};
}
void PackageResolver::resolve_package_dependencies(
    const std::filesystem::path &pkg_root) {
  auto manifest_path = pkg_root / "falcon.yml";
  if (!std::filesystem::exists(manifest_path)) {
    return;
  }

  auto manifest = PackageManifest::load(manifest_path);

  for (const auto &dep : manifest.dependencies) {
    try {
      std::string import_path;
      if (dep.github) {
        import_path = *dep.github;
      } else if (dep.local_path) {
        import_path = *dep.local_path;
      } else {
        continue;
      }

      auto resolved = resolve(import_path, pkg_root / "dummy.fal");

      resolve_package_dependencies(resolved.package_root);
    } catch (const std::exception &e) {
    }
  }
}

} // namespace falcon::pm
