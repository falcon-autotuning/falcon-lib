# Falcon Autotuner Language Tutorial

## Introduction

This tutorial will guide you through creating your first autotuner from scratch using the current Falcon DSL. By the end you will have a working voltage-sweep autotuner backed by a real C++ hardware wrapper, running entirely through `falcon-run` — no CMake, no manual compilation.

## Prerequisites

- Falcon DSL installed (`cd dsl && make install`)
- Basic understanding of state machines

---

## Tutorial: Voltage Sweep Autotuner

We'll build an autotuner that sweeps a voltage range and measures current at each step.

---

### Step 1: Define the State Machine

Create `voltage_sweep.fal`:

```fal
// voltage_sweep.fal
//
// Sweeps voltage from min_voltage to max_voltage in steps of 'step',
// measuring current at each point.  Saves results and reports the count.

ffimport "voltage_sweep_impl.cpp" () ()

autotuner VoltageSweep (float min_voltage, float max_voltage, float step) -> (int measurement_count) {
    float current_voltage = min_voltage;
    measurement_count = 0;
    start -> initialize;

    state initialize {
        bool init_success = initialize_device();
        if (init_success == true) { -> measure_point; }
        else                      { -> error_state;   }
    }

    state measure_point {
        float measured_current = measure_iv(current_voltage);
        measurement_count = measurement_count + 1;

        current_voltage = current_voltage + step;
        if (current_voltage <= max_voltage) { -> measure_point; }
        else                               { -> finalize;       }
    }

    state finalize {
        bool save_success = save_data(measurement_count);
        if (save_success == true) { -> complete;    }
        else                      { -> error_state; }
    }

    state complete    { terminal; }
    state error_state { terminal; }
}
```

Key points:

- The `ffimport` line at the top tells the engine to compile `voltage_sweep_impl.cpp` automatically — no separate build step is needed.
- The `uses` keyword is not part of the language. Cross-module calls simply use the `Module::symbol` qualified form.
- Inputs and outputs are declared directly in the autotuner signature.
- There are no `params {}` or `temp {}` blocks — variables are declared inline.

---

### Step 2: Implement Measurements in C++

Create `voltage_sweep_impl.cpp` **in the same directory** as the `.fal` file:

```cpp
// voltage_sweep_impl.cpp
//
// Falcon C ABI wrapper for the voltage sweep measurement routines.
// The engine compiles this file automatically via ffimport.

#include <falcon-typing/falcon_ffi.h>
#include <falcon-typing/FFIHelpers.hpp>
#include <iostream>
#include <fstream>
#include <vector>

// ── Mock hardware ─────────────────────────────────────────────────────────────

class MockDevice {
public:
    bool  initialize()           { return true; }
    void  set_voltage(double v)  { voltage_ = v; }
    double measure_current()     { return voltage_ * 1e-6; }  // Ohm's law mock
private:
    double voltage_ = 0.0;
};

static MockDevice g_device;
static std::vector<std::pair<double, double>> g_measurements;

// ── Falcon C ABI wrappers ─────────────────────────────────────────────────────

extern "C" void initialize_device(
    const FalconParamEntry* /*in*/,  int32_t /*in_count*/,
    FalconResultSlot*        out,    int32_t* out_count)
{
    std::cout << "Initializing device...\n";
    g_measurements.clear();
    bool ok = g_device.initialize();
    falcon::typing::ffi::engine::set_result(out, out_count, 0, ok);
}

extern "C" void measure_iv(
    const FalconParamEntry* in,  int32_t in_count,
    FalconResultSlot*       out, int32_t* out_count)
{
    auto params   = falcon::typing::ffi::engine::unpack_params(in, in_count);
    double voltage = std::get<double>(params.at("current_voltage"));

    g_device.set_voltage(voltage);
    double current = g_device.measure_current();

    std::cout << "  V=" << voltage << "  I=" << current << "\n";
    g_measurements.push_back({voltage, current});

    falcon::typing::ffi::engine::set_result(out, out_count, 0, current);
}

extern "C" void save_data(
    const FalconParamEntry* in,  int32_t in_count,
    FalconResultSlot*       out, int32_t* out_count)
{
    auto params = falcon::typing::ffi::engine::unpack_params(in, in_count);
    int64_t count = std::get<int64_t>(params.at("measurement_count"));

    std::cout << "Saving " << count << " measurement(s)...\n";

    std::ofstream f("iv_curve.csv");
    f << "Voltage (V),Current (A)\n";
    for (auto& [v, i] : g_measurements)
        f << v << "," << i << "\n";
    f.close();

    std::cout << "Saved to iv_curve.csv\n";
    falcon::typing::ffi::engine::set_result(out, out_count, 0, true);
}
```

The engine discovers and compiles this file when `voltage_sweep.fal` is loaded. There is nothing else to build.

---

### Step 3: Run

```bash
falcon-run VoltageSweep voltage_sweep.fal \
    --param min_voltage=0.0 \
    --param max_voltage=1.0 \
    --param step=0.1
```

Expected output:

```
Initializing device...
  V=0    I=0
  V=0.1  I=1e-07
  V=0.2  I=2e-07
  V=0.3  I=3e-07
  V=0.4  I=4e-07
  V=0.5  I=5e-07
  V=0.6  I=6e-07
  V=0.7  I=7e-07
  V=0.8  I=8e-07
  V=0.9  I=9e-07
  V=1    I=1e-06
Saving 11 measurement(s)...
Saved to iv_curve.csv

Autotuner 'VoltageSweep' completed.
Results (1):
  [0] 11
```

---

### Step 4: Add a Snapshot (Optional)

If you want to pre-seed parameters from a JSON snapshot instead of passing `--param` flags, create `snapshot.json` next to the `.fal` file:

```json
{
  "min_voltage": 0.0,
  "max_voltage": 1.0,
  "step": 0.1
}
```

Then run:

```bash
falcon-run VoltageSweep voltage_sweep.fal --snapshot snapshot.json
```

---

### Step 5: Embed in C++

If you need to call the autotuner from a C++ application:

```cpp
#include <falcon-dsl/AutotunerEngine.hpp>

int main() {
    falcon::dsl::AutotunerEngine engine;
    engine.load_fal_file("voltage_sweep.fal");   // ffimport compiled here

    falcon::typing::ParameterMap inputs;
    inputs["min_voltage"] = 0.0;
    inputs["max_voltage"] = 1.0;
    inputs["step"]        = 0.1;

    auto results = engine.run_autotuner("VoltageSweep", inputs);
    int64_t count = std::get<int64_t>(results[0]);
    std::cout << "Measured " << count << " point(s).\n";
    return 0;
}
```

No `cmake`, no custom build step, no shared library to produce by hand — `load_fal_file` handles everything.

---

## Step 6: Write a Test

Use `falcon-test` to regression-test the autotuner:

```fal
// voltage_sweep_tests.fal
import "/opt/falcon/libs/testing/testing.fal";

// The implementation under test
ffimport "voltage_sweep_impl.cpp" () ()

autotuner VoltageSweepTests -> (int passed, int failed) {
    passed = 0; failed = 0;
    start -> __init;

    state test_basic_sweep (TestRunner runner, TestContext t) {
        // 0.0, 0.5, 1.0 → 3 points
        int count = VoltageSweep(0.0, 1.0, 0.5);
        Error err = t.ExpectIntEq(count, 3, "3 measurements for 0→1 step 0.5");
        -> __end(runner, t);
    }

    state test_single_point (TestRunner runner, TestContext t) {
        int count = VoltageSweep(0.5, 0.5, 0.1);
        Error err = t.ExpectIntEq(count, 1, "single point when begin == end");
        -> __end(runner, t);
    }
}
```

```bash
falcon-test voltage_sweep_tests.fal
```

---

## Next Steps

- Add error handling states and retry logic
- Use [generic structs](LANGUAGE_REFERENCE.md#generic-structs) to build reusable data structures (e.g. `Accumulator<float>` for running statistics)
- Use [cross-module imports](LANGUAGE_REFERENCE.md#imports-and-modules) to share routines across multiple autotuners
- Use `falcon-test` with fixture `setup`/`teardown` for hardware-in-the-loop tests
- See [libs/testing/README.md](libs/testing/README.md) for the full testing library reference

See [LANGUAGE_REFERENCE.md](LANGUAGE_REFERENCE.md) for the complete language specification.
