#include "falcon-autotuner/PluginLoader.hpp"
#include <filesystem>
#include <gtest/gtest.h>

using namespace falcon::autotuner;

// Note: These tests require compiled plugins
// They may be skipped if plugins don't exist

TEST(PluginLoaderTest, LoadPlugin) {
  std::filesystem::path plugin_path = "plugins/libexample_plugin.so";

#ifdef _WIN32
  plugin_path = "plugins/example_plugin.dll";
#endif

  if (!std::filesystem::exists(plugin_path)) {
    GTEST_SKIP() << "Plugin not found: " << plugin_path;
  }

  try {
    auto autotuner = PluginLoader::load(plugin_path);
    ASSERT_NE(autotuner, nullptr);
    EXPECT_FALSE(autotuner->name().empty());
    EXPECT_EQ(autotuner->name(), "ExamplePlugin");

    // Try to run it
    auto result = autotuner->run();
    EXPECT_TRUE(result.success);

  } catch (const std::exception &e) {
    FAIL() << "Failed to load plugin: " << e.what();
  }
}

TEST(PluginLoaderTest, LoadMetadata) {
  std::filesystem::path plugin_path = "plugins/libexample_plugin.so";

#ifdef _WIN32
  plugin_path = "plugins/example_plugin.dll";
#endif

  if (!std::filesystem::exists(plugin_path)) {
    GTEST_SKIP() << "Plugin not found: " << plugin_path;
  }

  try {
    auto metadata = PluginLoader::load_metadata(plugin_path);
    EXPECT_EQ(metadata.api_version, AUTOTUNER_PLUGIN_API_VERSION);
    EXPECT_EQ(metadata.name, "ExamplePlugin");
    EXPECT_EQ(metadata.version, "1.0.0");
    EXPECT_FALSE(metadata.description.empty());
    EXPECT_FALSE(metadata.author.empty());
  } catch (const std::exception &e) {
    FAIL() << "Failed to load metadata: " << e.what();
  }
}

TEST(PluginLoaderTest, NonexistentPlugin) {
  EXPECT_THROW(PluginLoader::load("nonexistent_plugin.so"), std::runtime_error);
}

TEST(PluginLoaderTest, PluginMetadataConversion) {
  PluginMetadata cpp_meta;
  cpp_meta.api_version = 1;
  cpp_meta.name = "TestPlugin";
  cpp_meta.version = "2.0.0";
  cpp_meta.description = "Test description";
  cpp_meta.author = "Test Author";

  // Convert to C
  auto c_meta = cpp_meta.to_c();

  // Convert back to C++
  auto cpp_meta2 = PluginMetadata::from_c(c_meta);

  EXPECT_EQ(cpp_meta2.api_version, cpp_meta.api_version);
  EXPECT_EQ(cpp_meta2.name, cpp_meta.name);
  EXPECT_EQ(cpp_meta2.version, cpp_meta.version);
  EXPECT_EQ(cpp_meta2.description, cpp_meta.description);
  EXPECT_EQ(cpp_meta2.author, cpp_meta.author);
}

TEST(PluginLoaderTest, LongStringsAreTruncated) {
  PluginMetadata cpp_meta;
  cpp_meta.name = std::string(200, 'A'); // Longer than buffer

  auto c_meta = cpp_meta.to_c();
  auto cpp_meta2 = PluginMetadata::from_c(c_meta);

  // Should be truncated to fit in buffer
  EXPECT_LT(cpp_meta2.name.length(), 128);
  EXPECT_EQ(cpp_meta2.name.length(), 127); // Max length with null terminator
}
