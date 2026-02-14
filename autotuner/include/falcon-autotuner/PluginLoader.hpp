#pragma once

#include "falcon-autotuner/Autotuner.hpp"
#include <boost/dll/import.hpp>
#include <boost/dll/shared_library.hpp>
#include <boost/function.hpp>
#include <cstring>
#include <filesystem>

namespace falcon::autotuner {

/**
 * @brief Plugin API version
 */
constexpr int AUTOTUNER_PLUGIN_API_VERSION = 1;

/**
 * @brief C-compatible plugin metadata (POD type)
 */
struct PluginMetadataC {
  int api_version;
  std::array<char, 128> name;
  std::array<char, 32> version;
  std::array<char, 256> description;
  std::array<char, 128> author;
};

/**
 * @brief C++ wrapper for plugin metadata
 */
struct PluginMetadata {
  int api_version = AUTOTUNER_PLUGIN_API_VERSION;
  std::string name;
  std::string version;
  std::string description;
  std::string author;

  /**
   * @brief Convert from C struct
   */
  static PluginMetadata from_c(const PluginMetadataC &c_meta) {
    PluginMetadata meta;
    meta.api_version = c_meta.api_version;
    meta.name = std::string(c_meta.name.data());
    meta.version = std::string(c_meta.version.data());
    meta.description = std::string(c_meta.description.data());
    meta.author = std::string(c_meta.author.data());
    return meta;
  }

  /**
   * @brief Convert to C struct
   */
  [[nodiscard]] PluginMetadataC to_c() const {
    PluginMetadataC c_meta;
    c_meta.api_version = api_version;

    std::strncpy(c_meta.name.data(), name.c_str(), c_meta.name.size() - 1);
    c_meta.name[c_meta.name.size() - 1] = '\0';

    std::strncpy(c_meta.version.data(), version.c_str(),
                 c_meta.version.size() - 1);
    c_meta.version[c_meta.version.size() - 1] = '\0';

    std::strncpy(c_meta.description.data(), description.c_str(),
                 c_meta.description.size() - 1);
    c_meta.description[c_meta.description.size() - 1] = '\0';

    std::strncpy(c_meta.author.data(), author.c_str(),
                 c_meta.author.size() - 1);
    c_meta.author[c_meta.author.size() - 1] = '\0';

    return c_meta;
  }
};

/**
 * @brief C function signatures for plugin interface
 *
 * Plugins export these C functions:
 * - extern "C" void get_plugin_metadata(PluginMetadataC* out_metadata)
 * - extern "C" void* create_autotuner()
 * - extern "C" void destroy_autotuner(void* autotuner)
 */

/**
 * @brief Plugin interface
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
      auto lib = std::make_shared<dll::shared_library>(plugin_path.string());

      // Get metadata function (C interface)
      auto get_metadata_func = dll::import_alias<void(PluginMetadataC *)>(
          *lib, "get_plugin_metadata");

      PluginMetadataC c_metadata;
      get_metadata_func(&c_metadata);
      PluginMetadata metadata = PluginMetadata::from_c(c_metadata);

      // Check API version
      if (metadata.api_version != AUTOTUNER_PLUGIN_API_VERSION) {
        throw std::runtime_error("Plugin API version mismatch: expected " +
                                 std::to_string(AUTOTUNER_PLUGIN_API_VERSION) +
                                 ", got " +
                                 std::to_string(metadata.api_version));
      }

      // Get factory function (returns void*)
      auto create_func = dll::import_alias<void *()>(*lib, "create_autotuner");

      // Get destroy function
      auto destroy_func =
          dll::import_alias<void(void *)>(*lib, "destroy_autotuner");

      // Create the autotuner
      void *raw_ptr = create_func();
      if (!raw_ptr) {
        throw std::runtime_error("Plugin create_autotuner returned null");
      }

      // Wrap in shared_ptr with custom deleter that keeps library alive
      auto autotuner =
          std::shared_ptr<Autotuner>(static_cast<Autotuner *>(raw_ptr),
                                     [lib, destroy_func](Autotuner *ptr) {
                                       if (ptr) {
                                         destroy_func(static_cast<void *>(ptr));
                                       }
                                     });

      return autotuner;

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
    auto get_metadata_func =
        dll::import_alias<void(PluginMetadataC *)>(lib, "get_plugin_metadata");

    PluginMetadataC c_metadata;
    get_metadata_func(&c_metadata);
    return PluginMetadata::from_c(c_metadata);
  }
};

/**
 * @brief Helper macro for plugin development
 *
 * This macro handles all the C linkage complexity for you.
 * Just provide the plugin info and a factory function.
 *
 * Example:
 *
 * std::shared_ptr<Autotuner> create_my_autotuner() {
 *     auto tuner = std::make_shared<Autotuner>("MyAutotuner");
 *     // ... configure autotuner ...
 *     return tuner;
 * }
 *
 * FALCON_AUTOTUNER_PLUGIN(
 *     "MyPlugin",
 *     "1.0.0",
 *     "Description of my plugin",
 *     "Author Name",
 *     create_my_autotuner
 * )
 */
#define FALCON_AUTOTUNER_PLUGIN(NAME, VERSION, DESCRIPTION, AUTHOR,            \
                                FACTORY_FUNC)                                  \
  extern "C" {                                                                 \
  BOOST_SYMBOL_EXPORT void                                                     \
  get_plugin_metadata(::falcon::autotuner::PluginMetadataC *out_metadata) {    \
    if (!out_metadata)                                                         \
      return;                                                                  \
    ::falcon::autotuner::PluginMetadata meta;                                  \
    meta.api_version = ::falcon::autotuner::AUTOTUNER_PLUGIN_API_VERSION;      \
    meta.name = NAME;                                                          \
    meta.version = VERSION;                                                    \
    meta.description = DESCRIPTION;                                            \
    meta.author = AUTHOR;                                                      \
    *out_metadata = meta.to_c();                                               \
  }                                                                            \
                                                                               \
  BOOST_SYMBOL_EXPORT void *create_autotuner() {                               \
    try {                                                                      \
      auto autotuner_ptr = FACTORY_FUNC();                                     \
      if (!autotuner_ptr) {                                                    \
        return nullptr;                                                        \
      }                                                                        \
      return new std::shared_ptr<::falcon::autotuner::Autotuner>(              \
          autotuner_ptr);                                                      \
    } catch (...) {                                                            \
      return nullptr;                                                          \
    }                                                                          \
  }                                                                            \
                                                                               \
  BOOST_SYMBOL_EXPORT void destroy_autotuner(void *autotuner) {                \
    if (autotuner) {                                                           \
      auto ptr =                                                               \
          static_cast<std::shared_ptr<::falcon::autotuner::Autotuner> *>(      \
              autotuner);                                                      \
      delete ptr;                                                              \
    }                                                                          \
  }                                                                            \
  }

} // namespace falcon::autotuner
