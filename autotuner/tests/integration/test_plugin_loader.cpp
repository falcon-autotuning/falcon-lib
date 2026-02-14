#include "falcon-autotuner/PluginLoader.hpp"
#include <filesystem>
#include <gtest/gtest.h>

using namespace falcon::autotuner;

// Note: These tests require compiled plugins
// They may be skipped if plugins don't exist

TEST(PluginLoaderTest, DISABLED_LoadPlugin) {
  // This test is disabled by default as it requires a built plugin
  std::filesystem::path plugin_path = "plugins/libexample_plugin.so";

  if (!std::filesystem::exists(plugin_path)) {
    GTEST_SKIP() << "Plugin not found: " << plugin_path;
  }

  try {
    auto autotuner = PluginLoader::load(plugin_path);
    EXPECT_NE(autotuner, nullptr);
    EXPECT_FALSE(autotuner->name().empty());
  } catch (const std::exception &e) {
    FAIL() << "Failed to load plugin: " << e.what();
  }
}

TEST(PluginLoaderTest, DISABLED_LoadMetadata) {
  std::filesystem::path plugin_path = "plugins/libexample_plugin.so";

  if (!std::filesystem::exists(plugin_path)) {
    GTEST_SKIP() << "Plugin not found: " << plugin_path;
  }

  try {
    auto metadata = PluginLoader::load_metadata(plugin_path);
    EXPECT_EQ(metadata.api_version, AUTOTUNER_PLUGIN_API_VERSION);
    EXPECT_FALSE(metadata.name.empty());
  } catch (const std::exception &e) {
    FAIL() << "Failed to load metadata: " << e.what();
  }
}

TEST(PluginLoaderTest, NonexistentPlugin) {
  EXPECT_THROW(PluginLoader::load("nonexistent_plugin.so"), std::runtime_error);
}
