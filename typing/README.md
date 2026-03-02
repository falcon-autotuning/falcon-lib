# falcon-typing

Shared primitive-types and FFI-boundary library for the Falcon DSL ecosystem.

This module extracts the C-ABI types and C++ value model that are shared across
all Falcon components — the autotuner DSL, user-written measurement libraries,
and any future language bindings — into a single, independently installable
shared library (`libfalcon-typing.so`).

---

## Contents

| File | Purpose |
|------|---------|
| `include/falcon-typing/PrimitiveTypes.hpp` | `RuntimeValue`, `ParameterMap`, `FunctionResult`, `StructInstance`, `TupleValue`, `ErrorObject` |
| `include/falcon-typing/falcon_ffi.h` | Pure-C ABI structs (`FalconParamEntry`, `FalconResultSlot`, `FalconFFIFunc`) |
| `include/falcon-typing/FFIHelpers.hpp` | C++ helpers for crossing the C-ABI boundary in both directions |
| `src/PrimitiveTypes.cpp` | Implementations of `get_runtime_type_name` and `runtime_value_to_string` |

---

## Quick Start

### 1. Build & Install

```bash
# From falcon-lib root
make typing           # shortcut if defined in the root Makefile, OR:

cd typing
make install          # builds release + installs to /opt/falcon
```

### 2. Use in another project

```cmake
find_package(falcon-typing CONFIG REQUIRED
  PATHS /opt/falcon/lib/cmake/falcon-typing
)

target_link_libraries(my-target PRIVATE falcon::falcon-typing)
```

```cpp
#include "falcon-typing/PrimitiveTypes.hpp"
#include "falcon-typing/FFIHelpers.hpp"

// Create a ParameterMap, pack it across the C-ABI, unpack on the other side.
falcon::typing::ParameterMap params;
params["voltage"] = double{0.5};
params["label"]   = std::string{"gate_q0"};

auto packed  = falcon::typing::ffi::engine::pack_params(params);
// ... call FFI function ...
```

---

## API Overview

### `PrimitiveTypes.hpp`

#### `RuntimeValue`

A `std::variant` holding any value that can flow through the Falcon DSL:

| Alternative | DSL type |
|-------------|----------|
| `int64_t` | `int` |
| `double` | `float` |
| `bool` | `bool` |
| `std::string` | `string` |
| `std::nullptr_t` | `nil` |
| `ErrorObject` | `Error` |
| `std::shared_ptr<TupleValue>` | tuple |
| `std::shared_ptr<StructInstance>` | user struct |
| `ConnectionSP`, `ConnectionsSP`, `QuantitySP`, `GnameSP` | falcon_core physics types |

#### `ParameterMap`

```cpp
using ParameterMap = std::map<std::string, RuntimeValue>;
```

Named parameter bag used for both input and output of measurement routines.

#### `FunctionResult`

```cpp
using FunctionResult = std::vector<RuntimeValue>;
```

Ordered list of output values from a measurement function.

#### `StructInstance`

Represents a live instance of a FAL-defined struct.  
For FFI-bound structs, `native_handle` holds a type-erased `shared_ptr<void>`
to the real C++ object.

#### Helper functions

```cpp
std::string get_runtime_type_name(const RuntimeValue &);
std::string runtime_value_to_string(const RuntimeValue &);
```

---

### `falcon_ffi.h`

Pure-C header defining the binary ABI. Safe to include from both C and C++.

- `FalconTypeTag` — enum of supported primitive types
- `FalconParamEntry` — one named parameter (key + tagged union value)
- `FalconResultSlot` — one positional result (tagged union value)
- `FalconFFIFunc` — function pointer type every wrapper must conform to

---

### `FFIHelpers.hpp`

C++ template helpers that bridge `RuntimeValue` ↔ C ABI.

#### `falcon::typing::ffi::engine` namespace (engine side)

| Function | Description |
|----------|-------------|
| `pack_params(ParameterMap)` | Pack params into `FalconParamEntry[]` before a call |
| `unpack_results(FalconResultSlot*, int32_t)` | Unpack results after a call; frees wrapper-allocated memory |

#### `falcon::typing::ffi::wrapper` namespace (wrapper side)

| Function | Description |
|----------|-------------|
| `unpack_params(FalconParamEntry*, int32_t)` | Reconstruct a `ParameterMap` from the C array |
| `get_opaque<T>(entries, count, key)` | Extract a typed `shared_ptr<T>` from an opaque entry |
| `pack_results(FunctionResult, slots, capacity, out_count)` | Pack results into `FalconResultSlot[]`; heap-allocates strings |
| `pack_single(RuntimeValue, slots, out_count)` | Convenience wrapper for a single result value |

---

## Building from Source

```bash
cd typing
mkdir -p build/release && cd build/release
cmake ../.. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=../../.vcpkg/scripts/buildsystems/vcpkg.cmake \
  -Dfalcon_core_DIR=/opt/falcon/lib/cmake/falcon_core \
  -G Ninja
ninja
sudo cmake --install . --prefix /opt/falcon
```

## Running Tests

```bash
cd typing
make test              # release
make test-debug        # debug
make test-verbose      # verbose output
```

Tests exercise every primitive type in a full engine→wrapper→engine round-trip
across the C-ABI boundary using Google Test.

## Uninstalling

```bash
cd typing
make uninstall
```

---

## License

MPL-2.0
