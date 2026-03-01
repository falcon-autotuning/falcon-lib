#include "falcon-pm/PackageManager.hpp"
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

static void print_usage() {
  std::cout <<
      R"(falcon-pm — Falcon package manager

USAGE:
  falcon-pm <command> [args]

COMMANDS:
  init  [dir] [name]        Create falcon.yml and .falcon/cache/ in <dir>
                            (defaults to current directory)
  install <source> [ver]    Install a package from a local path or GitHub
  remove  <name>            Remove a package from the manifest and cache
  list                      List all packages in the cache index
  help                      Show this message
)" << '\n';
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    print_usage();
    return 0;
  }

  std::string cmd = argv[1];

  try {
    if (cmd == "help" || cmd == "--help" || cmd == "-h") {
      print_usage();
      return 0;
    }

    if (cmd == "init") {
      std::filesystem::path dir =
          (argc >= 3) ? argv[2] : std::filesystem::current_path();
      std::string name = (argc >= 4) ? argv[3] : dir.filename().string();
      falcon::pm::PackageManager::init(dir, name);
      std::cout << "Initialized Falcon package '" << name << "' in "
                << dir.string() << '\n';
      return 0;
    }

    // For all other commands we need an existing project context.
    falcon::pm::PackageManager pm(std::filesystem::current_path());

    if (cmd == "list") {
      auto pkgs = pm.list();
      if (pkgs.empty()) {
        std::cout << "(no packages cached)\n";
      } else {
        std::cout << std::left;
        std::cout.width(14);
        std::cout << "SHA (prefix)";
        std::cout.width(12);
        std::cout << "Version";
        std::cout << "Cached path\n";
        std::cout << std::string(60, '-') << '\n';
        for (const auto &p : pkgs) {
          std::cout.width(14);
          std::cout << p.name;
          std::cout.width(12);
          std::cout << p.version;
          std::cout << p.cached_path.string() << '\n';
        }
      }
      return 0;
    }

    if (cmd == "install") {
      if (argc < 3) {
        std::cerr << "Usage: falcon-pm install <source> [version]\n";
        return 1;
      }
      std::string source = argv[2];
      std::string version = (argc >= 4) ? argv[3] : "*";
      pm.install(source, version);
      std::cout << "Installed: " << source << " (" << version << ")\n";
      return 0;
    }

    if (cmd == "remove") {
      if (argc < 3) {
        std::cerr << "Usage: falcon-pm remove <package-name>\n";
        return 1;
      }
      pm.remove(argv[2]);
      std::cout << "Removed: " << argv[2] << '\n';
      return 0;
    }

    std::cerr << "Unknown command: " << cmd << '\n';
    print_usage();
    return 1;

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << '\n';
    return 1;
  }
}
