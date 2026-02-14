#include "falcon-autotuner/Autotuner.hpp"
#include "falcon-autotuner/PluginLoader.hpp"

using namespace falcon::autotuner;

class ExampleMeasurement : public MeasurementRoutine {
public:
  [[nodiscard]] std::string name() const override {
    return "example_measurement";
  }
  MeasurementResult execute(const ParameterMap &inputs) override {
    MeasurementResult result;
    if (inputs.has("input")) {
      int value = inputs.get<int>("input");
      result.outputs.set("output", value * 2);
    }
    return result;
  }
};
std::shared_ptr<Autotuner> create_example_autotuner() {
  auto autotuner = std::make_shared<Autotuner>("ExamplePlugin");

  // Create a simple measurement
  auto measurement = std::make_shared<ExampleMeasurement>();

  // Iterate over values 1-10
  auto root = Autotuner::iterate_range("input", 1, 11, 1,
                                       Autotuner::measurement(measurement));

  autotuner->set_root(root);
  return autotuner;
}

// Register plugin
FALCON_AUTOTUNER_PLUGIN("ExamplePlugin", "1.0.0",
                        "Simple example plugin that doubles input values",
                        "Falcon Team", create_example_autotuner)
