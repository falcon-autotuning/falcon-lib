#include "falcon-pm/PackageManager.hpp"
#include <filesystem>
#include <iostream>
#include <string>

static void print_usage() {
  std::cout <<
      R"(falcon-pm: A package manager for the Falcon DSL.

Usage:
  falcon-pm [options]

Options:
 --init  [dir] [name]        Create falcon.yml and .falcon/cache/ in <dir>
 --install <source> [ver]    Install a package from a local path or GitHub
 --remove  <name>            Remove a package from the manifest
 --build [extra_flags]       Compile FFI .cpp wrappers with extra flags
 --list                      List all packages in the cache index
 --help                      Show this message
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

    if (cmd == "--init") {
      std::filesystem::path dir =
          (argc >= 3) ? argv[2] : std::filesystem::current_path();
      std::string name = (argc >= 4) ? argv[3] : dir.filename().string();
      falcon::pm::PackageManager::init(dir, name);
      std::cout << "Initialized Falcon package '" << name << "' in "
                << dir.string() << '\n';
      return 0;
    }

    falcon::pm::PackageManager pm(std::filesystem::current_path());

    if (cmd == "build" || cmd == "--build") {
      std::string extra_flags;
      // Collect all arguments after "build" as extra flags
      for (int i = 2; i < argc; ++i) {
        extra_flags += std::string(argv[i]) + " ";
      }
      pm.build(pm.project_root(), extra_flags);
      return 0;
    }

    if (cmd == "list" || cmd == "--list") {
      auto pkgs = pm.list();
      if (pkgs.empty())
        std::cout << "(no packages cached)\n";
      else {
        for (const auto &p : pkgs) {
          std::cout << p.name << " (" << p.version << ") -> "
                    << p.cached_path.string() << '\n';
        }
      }
      return 0;
    }

    if (cmd == "--install") {
      if (argc < 3)
        return 1;
      pm.install(argv[2], (argc >= 4) ? argv[3] : "*");
      return 0;
    }

    if (cmd == "--remove") {
      if (argc < 3)
        return 1;
      pm.remove(argv[2]);
      return 0;
    }

    print_usage();
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << '\n';
    return 1;
  }
}
