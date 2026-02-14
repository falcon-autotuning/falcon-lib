#pragma once

#include "falcon-autotuner/Autotuner.hpp"
#include <boost/dll/import.hpp>
#include <boost/dll/shared_library.hpp>
#include <boost/function.hpp>
#include <filesystem>

namespace falcon::autotuner {

/**
 * @brief Plugin API version
 */
constexpr int AUTOTUNER_PLUGIN_API_VERSION = 1;

/**
 * @brief Plugin metadata
 */
struct PluginMetadata {
  int api_version = AUTOTUNER_PLUGIN_API_VERSION;
  std::string name;
  std::string version;
  std::string description;
  std::string author;
};

/**
 * @brief Plugin interface
 *
 * Plugins should export these functions:
 * - extern "C" PluginMetadata get_plugin_metadata()
 * - extern "C" std::shared_ptr<Autotuner> create_autotuner()
 */
class PluginLoader {
public:
  /**
   * @brief Load a plugin from a shared library
   */
  static std::shared_ptr<Autotuner>
  load(const std::filesystem::path &plugin_path) {
    namespace dll = boost::dll;

    if (!std::filesystem::exists(plugin_path)) {
      throw std::runtime_error("Plugin not found: " + plugin_path.string());
    }

    try {
      // Load the shared library
      dll::shared_library lib(plugin_path.string());

      // Get metadata function
      auto get_metadata =
          dll::import_alias<PluginMetadata()>(lib, "get_plugin_metadata");

      PluginMetadata metadata = get_metadata();

      // Check API version
      if (metadata.api_version != AUTOTUNER_PLUGIN_API_VERSION) {
        throw std::runtime_error("Plugin API version mismatch: expected " +
                                 std::to_string(AUTOTUNER_PLUGIN_API_VERSION) +
                                 ", got " +
                                 std::to_string(metadata.api_version));
      }

      // Get factory function
      auto create_autotuner = dll::import_alias<std::shared_ptr<Autotuner>()>(
          lib, "create_autotuner");

      return create_autotuner();

    } catch (const std::exception &e) {
      throw std::runtime_error(
          "Failed to load plugin: " + plugin_path.string() + " - " + e.what());
    }
  }

  /**
   * @brief Load plugin metadata without creating autotuner
   */
  static PluginMetadata
  load_metadata(const std::filesystem::path &plugin_path) {
    namespace dll = boost::dll;

    dll::shared_library lib(plugin_path.string());
    auto get_metadata =
        dll::import_alias<PluginMetadata()>(lib, "get_plugin_metadata");

    return get_metadata();
  }
};

/**
 * @brief Helper macro for plugin development
 */
#define FALCON_AUTOTUNER_PLUGIN(NAME, VERSION, DESCRIPTION, AUTHOR,            \
                                FACTORY_FUNC)                                  \
  extern "C" {                                                                 \
  BOOST_SYMBOL_EXPORT ::falcon::autotuner::PluginMetadata                      \
  get_plugin_metadata() {                                                      \
    ::falcon::autotuner::PluginMetadata meta;                                  \
    meta.api_version = ::falcon::autotuner::AUTOTUNER_PLUGIN_API_VERSION;      \
    meta.name = NAME;                                                          \
    meta.version = VERSION;                                                    \
    meta.description = DESCRIPTION;                                            \
    meta.author = AUTHOR;                                                      \
    return meta;                                                               \
  }                                                                            \
  BOOST_SYMBOL_EXPORT std::shared_ptr<::falcon::autotuner::Autotuner>          \
  create_autotuner() {                                                         \
    return FACTORY_FUNC();                                                     \
  }                                                                            \
  }

} // namespace falcon::autotuner
