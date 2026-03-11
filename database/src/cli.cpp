// falcon-db-cli: A command-line interface for the Falcon Database module.
//
// Usage:
//   falcon-db-cli [options] <command> [args...]
//
// Options:
//  --db <url>                  Override FALCON_DATABASE_URL environment
//  variable
//
// Commands:
//  init                        Initialize the database schema
//  schema                      Print the JSON schema for snapshots
//  snapshot export <file>      Export all characteristics to a JSON file
//  snapshot import <file>      Import characteristics from a JSON file
//          [--clear]           (Optional) Clear existing characteristics before
//          import
//  snapshot validate <file>    Validate a JSON file against the snapshot schema
//  help                        Show this message

#include "falcon-database/DatabaseConnection.hpp"
#include "falcon-database/SnapshotManager.hpp"
#include <iostream>
#include <string>
#include <vector>

static void print_usage() {
  std::cout
      << R"(falcon-db-cli: A command-line interface for the Falcon Database.

Usage:
  falcon-db-cli [options] <command> [args...]

Options:
  --db <url>                  Override FALCON_DATABASE_URL environment variable
                              (Otherwise, falls back to the environment variable)

Commands:
  init                        Initialize the database schema
  schema                      Print the JSON schema for snapshots
  snapshot export <file>      Export all characteristics to a JSON file
  snapshot import <file>      Import characteristics from a JSON file
          [--clear]           (Optional) Clear existing characteristics before import
  snapshot validate <file>    Validate a JSON file against the snapshot schema
  help                        Show this message
)" << '\n';
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    print_usage();
    return 0;
  }

  std::vector<std::string> args(argv + 1, argv + argc);
  std::string db_url = "";
  size_t cmd_idx = 0;

  // Parse global options
  while (cmd_idx < args.size() && args[cmd_idx].rfind("--", 0) == 0) {
    if (args[cmd_idx] == "--db") {
      if (cmd_idx + 1 >= args.size()) {
        std::cerr << "Error: --db requires an argument.\n";
        return 1;
      }
      db_url = args[cmd_idx + 1];
      cmd_idx += 2;
    } else {
      std::cerr << "Unknown option: " << args[cmd_idx] << '\n';
      print_usage();
      return 1;
    }
  }

  if (cmd_idx >= args.size()) {
    print_usage();
    return 0;
  }

  std::string cmd = args[cmd_idx];

  try {
    if (cmd == "help" || cmd == "--help" || cmd == "-h") {
      print_usage();
      return 0;
    }

    if (cmd == "schema") {
      std::cout << falcon::database::SnapshotManager::get_json_schema() << '\n';
      return 0;
    }

    if (cmd == "snapshot") {
      if (cmd_idx + 1 >= args.size()) {
        std::cerr << "Error: 'snapshot' requires a subcommand (export, import, "
                     "validate).\n";
        return 1;
      }
      std::string subcmd = args[cmd_idx + 1];

      if (subcmd == "validate") {
        if (cmd_idx + 2 >= args.size()) {
          std::cerr << "Usage: falcon-db-cli snapshot validate <file>\n";
          return 1;
        }
        std::string filename = args[cmd_idx + 2];
        bool valid =
            falcon::database::SnapshotManager::validate_snapshot(filename);
        if (valid) {
          std::cout << "Snapshot is valid: " << filename << '\n';
          return 0;
        } else {
          std::cerr << "Snapshot validation failed: " << filename << '\n';
          return 1;
        }
      }

      // Both export and import need a database connection
      auto conn =
          std::make_shared<falcon::database::AdminDatabaseConnection>(db_url);
      falcon::database::SnapshotManager manager(conn);

      if (subcmd == "export") {
        if (cmd_idx + 2 >= args.size()) {
          std::cerr << "Usage: falcon-db-cli snapshot export <file>\n";
          return 1;
        }
        std::string filename = args[cmd_idx + 2];
        manager.export_to_json(filename);
        return 0;
      }

      if (subcmd == "import") {
        if (cmd_idx + 2 >= args.size()) {
          std::cerr
              << "Usage: falcon-db-cli snapshot import <file> [--clear]\n";
          return 1;
        }
        std::string filename = args[cmd_idx + 2];
        bool clear_existing = false;
        if (cmd_idx + 3 < args.size() && args[cmd_idx + 3] == "--clear") {
          clear_existing = true;
        }
        manager.import_from_json(filename, clear_existing);
        return 0;
      }

      std::cerr << "Unknown snapshot command: " << subcmd << '\n';
      return 1;
    }

    if (cmd == "init") {
      falcon::database::AdminDatabaseConnection conn(db_url);
      conn.initialize_schema();
      std::cout << "Database schema initialized successfully.\n";
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
