#include "falcon-pm/PackageManifest.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <yaml-cpp/yaml.h>

namespace falcon::pm {

PackageManifest PackageManifest::load(const std::filesystem::path &path) {
  YAML::Node doc;
  try {
    doc = YAML::LoadFile(path.string());
  } catch (const YAML::Exception &e) {
    throw std::runtime_error("Failed to parse falcon.yml at '" + path.string() +
                             "': " + e.what());
  }

  PackageManifest m;
  m.name = doc["name"] ? doc["name"].as<std::string>() : "";
  m.version = doc["version"] ? doc["version"].as<std::string>() : "0.0.0";
  m.maintainer = doc["maintainer"] ? doc["maintainer"].as<std::string>() : "";
  m.github = doc["github"] ? doc["github"].as<std::string>() : "";

  // Load FFI wrappers list
  if (doc["ffi"] && doc["ffi"].IsSequence()) {
    for (const auto &wrapper_node : doc["ffi"]) {
      m.ffi.push_back(wrapper_node.as<std::string>());
    }
  }

  if (doc["dependencies"] && doc["dependencies"].IsSequence()) {
    for (const auto &dep_node : doc["dependencies"]) {
      Dependency d;
      d.name = dep_node["name"] ? dep_node["name"].as<std::string>() : "";
      d.version =
          dep_node["version"] ? dep_node["version"].as<std::string>() : "*";
      if (dep_node["github"])
        d.github = dep_node["github"].as<std::string>();
      if (dep_node["local_path"])
        d.local_path = dep_node["local_path"].as<std::string>();

      // Load package_info if present
      if (dep_node["package_info"]) {
        PackageInfo info;
        auto info_node = dep_node["package_info"];

        if (info_node["package_root"])
          info.package_root = info_node["package_root"].as<std::string>();

        if (info_node["main_file"])
          info.main_file = info_node["main_file"].as<std::string>();

        if (info_node["ffi_wrappers"] &&
            info_node["ffi_wrappers"].IsSequence()) {
          for (const auto &wrapper_node : info_node["ffi_wrappers"]) {
            info.ffi_wrappers.push_back(wrapper_node.as<std::string>());
          }
        }

        d.package_info = info;
      }

      m.dependencies.push_back(std::move(d));
    }
  }
  return m;
}

void PackageManifest::save(const std::filesystem::path &path) const {
  YAML::Emitter out;
  out << YAML::BeginMap;
  out << YAML::Key << "name" << YAML::Value << name;
  out << YAML::Key << "version" << YAML::Value << version;
  out << YAML::Key << "maintainer" << YAML::Value << maintainer;
  out << YAML::Key << "github" << YAML::Value << github;

  // Save FFI wrappers
  if (!ffi.empty()) {
    out << YAML::Key << "ffi" << YAML::Value << YAML::BeginSeq;
    for (const auto &wrapper : ffi) {
      out << wrapper;
    }
    out << YAML::EndSeq;
  }

  out << YAML::Key << "dependencies" << YAML::Value << YAML::BeginSeq;

  for (const auto &d : dependencies) {
    out << YAML::BeginMap;
    out << YAML::Key << "name" << YAML::Value << d.name;
    out << YAML::Key << "version" << YAML::Value << d.version;

    if (d.github)
      out << YAML::Key << "github" << YAML::Value << *d.github;
    if (d.local_path)
      out << YAML::Key << "local_path" << YAML::Value << *d.local_path;

    // Save package_info if present
    if (d.package_info) {
      out << YAML::Key << "package_info" << YAML::Value << YAML::BeginMap;

      if (d.package_info->package_root)
        out << YAML::Key << "package_root" << YAML::Value
            << *d.package_info->package_root;

      if (d.package_info->main_file)
        out << YAML::Key << "main_file" << YAML::Value
            << *d.package_info->main_file;

      if (!d.package_info->ffi_wrappers.empty()) {
        out << YAML::Key << "ffi_wrappers" << YAML::Value << YAML::BeginSeq;
        for (const auto &wrapper : d.package_info->ffi_wrappers) {
          out << wrapper;
        }
        out << YAML::EndSeq;
      }

      out << YAML::EndMap;
    }

    out << YAML::EndMap;
  }
  out << YAML::EndSeq;
  out << YAML::EndMap;

  std::ofstream f(path);
  if (!f.is_open())
    throw std::runtime_error("Cannot write falcon.yml to: " + path.string());
  f << out.c_str();
}

PackageManifest PackageManifest::make_empty(const std::string &pkg_name) {
  PackageManifest m;
  m.name = pkg_name;
  m.version = "0.1.0";
  m.maintainer = "";
  m.github = "";
  return m;
}

std::optional<std::filesystem::path>
PackageManifest::find_root(const std::filesystem::path &start) {
  auto dir = std::filesystem::absolute(start);
  if (std::filesystem::is_regular_file(dir))
    dir = dir.parent_path();

  while (true) {
    auto candidate = dir / "falcon.yml";
    if (std::filesystem::exists(candidate))
      return dir;
    auto parent = dir.parent_path();
    if (parent == dir)
      break; // filesystem root
    dir = parent;
  }
  return std::nullopt;
}

} // namespace falcon::pm
