# Falcon Autotuner Language Tutorial

## Introduction

This tutorial will guide you through creating your first autotuner from scratch.

## Prerequisites

- C++17 compiler
- CMake 3.20+
- Falcon Autotuner library installed
- Basic understanding of state machines

## Tutorial: Voltage Sweep Autotuner

We'll build an autotuner that sweeps voltage and measures current.

### Step 1: Define the State Machine

Create `voltage_sweep.fal`:

```fal
autotuner VoltageSweep {
  requires: [];
  
  params {
    float min_voltage = 0.0;
    float max_voltage = 1.0;
    float step = 0.1;
    float current_voltage = 0.0;
    int measurement_count = 0;
  }
  
  start -> initialize;
  
  state initialize {
    temp {
      bool init_success;
      string error_msg;
    }
    
    measurement: initialize_device();
    
    if (init_success == true) -> start_sweep;
    else -> error_state[error_msg: error];
  }
  
  state start_sweep {
    measurement: prepare_sweep();
    
    current_voltage = min_voltage;
    
    -> measure_point;
  }
  
  state measure_point {
    temp {
      float measured_current;
      bool valid;
    }
    
    measurement: measure_iv(current_voltage);
    
    if (valid == true) {
      measurement_count = measurement_count + 1;
      
      if (current_voltage + step < max_voltage) {
        current_voltage = current_voltage + step;
        -> measure_point;
      }
      else -> finalize;
    }
    else -> error_state;
  }
  
  state finalize {
    temp {
      bool save_success;
    }
    
    measurement: save_data(measurement_count);
    
    if (save_success == true) -> complete;
    else -> error_state;
  }
  
  state complete {
    terminal;
  }
  
  state error_state {
    params {
      string error = "Unknown error";
    }
    terminal;
  }
}
```

---

## Step 2: Implement Measurements in C++

Create ==voltage_sweep_impl.cpp==:

```cpp
#include "falcon-autotuner/MeasurementRoutine.hpp"
#include <iostream>
#include <fstream>
#include <vector>

// Mock hardware interface
class MockDevice {
public:
    bool initialize() { return true; }
    void set_voltage(float v) { voltage_ = v; }
    float measure_current() { return voltage_ * 1e-6f; }  // Ohm's law
private:
    float voltage_ = 0.0f;
};

static MockDevice g_device;
static std::vector<std::pair<float, float>> g_measurements;

namespace VoltageSweep {

using namespace falcon::autotuner;

ParameterMap initialize_device(const ParameterMap& params) {
    ParameterMap result;
    
    std::cout << "Initializing device...\n";
    
    bool success = g_device.initialize();
    g_measurements.clear();
    
    result.set("init_success", success);
    result.set("error_msg", success ? std::string("") : std::string("Init failed"));
    
    return result;
}

ParameterMap prepare_sweep(const ParameterMap& params) {
    ParameterMap result;
    
    float min_v = params.get<float>("min_voltage");
    float max_v = params.get<float>("max_voltage");
    
    std::cout << "Preparing sweep: " << min_v << " to " << max_v << " V\n";
    
    return result;  // No output params needed
}

ParameterMap measure_iv(const ParameterMap& params) {
    ParameterMap result;
    
    float voltage = params.get<float>("current_voltage");
    
    std::cout << "Measuring at " << voltage << " V... ";
    
    g_device.set_voltage(voltage);
    float current = g_device.measure_current();
    
    std::cout << current << " A\n";
    
    g_measurements.push_back({voltage, current});
    
    result.set("measured_current", current);
    result.set("valid", true);
    
    return result;
}

ParameterMap save_data(const ParameterMap& params) {
    ParameterMap result;
    
    int count = params.get<int>("measurement_count");
    
    std::cout << "Saving " << count << " measurements...\n";
    
    std::ofstream file("iv_curve.csv");
    file << "Voltage (V),Current (A)\n";
    for (const auto& [v, i] : g_measurements) {
        file << v << "," << i << "\n";
    }
    file.close();
    
    std::cout << "Saved to iv_curve.csv\n";
    
    result.set("save_success", true);
    
    return result;
}

} // namespace VoltageSweep
```

---

## Step 3: Compile to Shared Library

Create ==CMakeLists.txt==:

```cpp
cmake_minimum_required(VERSION 3.20)
project(voltage_sweep_autotuner)

set(CMAKE_CXX_STANDARD 17)

find_package(falcon-autotuner REQUIRED)

# Compile .fal file
add_custom_command(
    OUTPUT 
        ${CMAKE_CURRENT_BINARY_DIR}/GeneratedAutotuners.hpp
        ${CMAKE_CURRENT_BINARY_DIR}/GeneratedAutotuners.cpp
    COMMAND falc ${CMAKE_CURRENT_SOURCE_DIR}/voltage_sweep.fal 
            -o ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS voltage_sweep.fal
    COMMENT "Compiling voltage_sweep.fal"
)

# Build shared library
add_library(voltage_sweep_autotuner SHARED
    voltage_sweep_impl.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/GeneratedAutotuners.cpp
)

target_include_directories(voltage_sweep_autotuner
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(voltage_sweep_autotuner
    PRIVATE falcon::autotuner-core
)
```

---

## Step 4: Create Snapshot File

Create ==snapshot.json==:

```json
{
  "min_voltage": 0.0,
  "max_voltage": 1.0,
  "step": 0.1,
  "current_voltage": 0.0,
  "measurement_count": 0
}
```

---

## Step 5: Build

```bash
mkdir build
cd build
cmake ..
make
```

---
This produces: ==libvoltage_sweep_autotuner.so==

## Step 6: Run

```bash
falcon-runtime libvoltage_sweep_autotuner.so \
    --entry VoltageSweep \
    --snapshot snapshot.json
```

Output:

```Code
Loading library: libvoltage_sweep_autotuner.so
Entry autotuner: VoltageSweep

=== Running Autotuner ===

Initializing device...
Preparing sweep: 0.0 to 1.0 V
Measuring at 0.0 V... 0.0 A
Measuring at 0.1 V... 1e-07 A
Measuring at 0.2 V... 2e-07 A
...
Measuring at 1.0 V... 1e-06 A
Saving 11 measurements...
Saved to iv_curve.csv

=== Results ===
✓ Success!
Final state: VoltageSweep::complete
Transitions: 14
```

---

## Next Steps

- Add error handling states
- Implement cross-autotuner transitions
- Use database integration for snapshots
- Build more complex state machines

See EXAMPLES.md for more examples.
