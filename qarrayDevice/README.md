# falcon-qarrayDevice

A C++ shared library that embeds the CPython interpreter and wraps the
`quantum_dot_device.py` `Device` class, exposing it natively to C++ consumers.

## Architecture

```
C++ caller
    │
    ▼
falcon::qarray::Device          (C++ class in libfalcon-qarrayDevice.so)
    │  uses pybind11::embed
    ▼
CPython interpreter              (started once per process)
    │  imports
    ▼
quantum_dot_device.py            (installed to /opt/falcon/share/qarrayDevice/)
    │  imports
    ▼
qarray  (Python package)  ◄── pip-installed
numpy, pyyaml             ◄── pip-installed
```

## Dependencies

### C++ (via vcpkg — already in root vcpkg.json)

- `pybind11 >= 2.13.1`
- `gtest` (tests only)

### Python version requirement

**Python 3.12 or earlier is required** for `make install-python-deps`.

`qarray` depends on `qarray-rust-core`, a Rust extension that compiles OSQP
from C source via CMake. Pre-built wheels are only published for Python ≤ 3.12
on Linux x86_64. On Python 3.13, pip/uv falls back to a source build that fails
because the vendored OSQP CMakeLists.txt predates CMake 4.0 (your system has
CMake 4.2.3).

The Makefile's `install-python-deps` target pins to Python 3.12 automatically.
To override: `make install-python-deps PYTHON_VERSION=3.11`

### System

- Python 3.9+ (with development headers — `python3-dev` on Debian/Ubuntu)
- A Python environment with the packages in `requirements.txt`

### Python (pip)

- `qarray >= 0.4.0`
- `numpy >= 1.24.0`
- `pyyaml >= 6.0`

## Building

```bash
# From the falcon-lib repo root — first time only
make deps
make install-vcpkg-deps
make install-core

# Then, from this directory:
cd qarrayDevice

# Install Python deps (once)
make install-python-deps

# Build & install
make build-release
make install

# Or from the repo root:
make -C qarrayDevice build-release
make -C qarrayDevice install
```

## Testing

```bash
cd qarrayDevice
make test
```

## Runtime: finding `quantum_dot_device.py`

At runtime the library looks for `quantum_dot_device.py` in this order:

1. `FALCON_QARRAY_PYTHON_PATH` environment variable (if set)
2. The compiled-in path: `/opt/falcon/share/qarrayDevice/` (set at cmake configure time via `CMAKE_INSTALL_PREFIX`)

Override example:

```bash
export FALCON_QARRAY_PYTHON_PATH=/my/custom/path
./my_cpp_binary
```

## C++ Usage Example

```cpp
#include <qarrayDevice/Device.hpp>
#include <iostream>

int main() {
    // Load device from a YAML config
    falcon::qarray::Device dev("/path/to/device_config.yaml");

    std::cout << "n_dots: " << dev.n_dots() << "\n";

    // Set some voltages
    dev.set_voltages({{"P1", 0.3}, {"P2", -0.1}});

    // 2D scan
    auto result = dev.scan_2d("P1", "P2", {-0.5, 0.5}, {-0.5, 0.5}, 50);
    std::cout << "charge_states size: " << result.charge_states.size() << "\n";

    if (result.has_sensor) {
        std::cout << "sensor_output size: " << result.sensor_output.size() << "\n";
    }

    return 0;
}
```

## Linking in your CMake project

```cmake
find_package(falcon-qarrayDevice CONFIG REQUIRED)
target_link_libraries(my_target PRIVATE falcon::falcon-qarrayDevice)
```
