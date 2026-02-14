#include "falcon-autotuner/Autotuner.hpp"
#include "falcon-autotuner/MeasurementRoutine.hpp"
#include "falcon-autotuner/PluginLoader.hpp"

using namespace falcon::autotuner;

/**
 * @brief Example measurement for the plugin
 */
class ExampleMeasurement : public MeasurementRoutine {
public:
  MeasurementResult execute(const ParameterMap &inputs) override {
    MeasurementResult result;

    // Example: multiply input by 2
    if (inputs.has("input")) {
      auto value = inputs.try_get<int64_t>("input");
      if (value) {
        result.outputs.set("output", *value * 2);
        result.outputs.set("operation", std::string("multiply_by_2"));
      }
    }

    return result;
  }

  [[nodiscard]] std::string name() const override {
    return "example_measurement";
  }

  [[nodiscard]] std::vector<std::string> expected_inputs() const override {
    return {"input"};
  }

  [[nodiscard]] std::vector<std::string> expected_outputs() const override {
    return {"output", "operation"};
  }
};

/**
 * @brief Factory function to create the autotuner
 *
 * This is the function referenced in the plugin macro.
 * It returns a shared_ptr<Autotuner>.
 */
std::shared_ptr<Autotuner> create_example_autotuner() {
  auto autotuner = std::make_shared<Autotuner>("ExamplePlugin");

  // Create measurement
  auto measurement = std::make_shared<ExampleMeasurement>();

  // Iterate over values 1-10
  auto root = Autotuner::iterate_range("input", 1, 11, 1,
                                       Autotuner::measurement(measurement));

  autotuner->set_root(root);
  return autotuner;
}

// Register plugin - this macro handles all the C linkage properly
FALCON_AUTOTUNER_PLUGIN("ExamplePlugin", "1.0.0",
                        "Simple example plugin that doubles input values",
                        "Falcon Team", create_example_autotuner)
