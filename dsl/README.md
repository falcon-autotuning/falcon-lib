# Falcon Autotuner

A domain-specific language and runtime for quantum device autotuning state machines.

## Quick Start

### 1. Install

```bash
# From falcon-lib root
make install-deps
make autotuner
sudo make -C autotuner install
```

```

### 2. Write a .fal file

See ==docs/LANGUAGE_REFERENCE.md== for complete syntax.
```fal 
autotuner MyAutotuner {
  params {
    int counter = 0;
  }
  
  start -> init;
  
  state init {
    measurement: initialize();
    if (success == true) -> done;
    else -> error;
  }
  
  state done { terminal; }
  state error { terminal; }
}
```

### 3. Implement measurements

```cpp
namespace MyAutotuner {
    ParameterMap initialize(const ParameterMap& params) {
        ParameterMap result;
        // Your code here
        result.set("success", true);
        return result;
    }
}
```

### 4. Build

```bash
falc my_autotuner.fal -o generated/
g++ -shared -fPIC impl.cpp generated/GeneratedAutotuners.cpp \
    -o libmy_autotuner.so -lfalcon-autotuner-core
```

### 5. Run

```bash
falcon-runtime libmy_autotuner.so \
    --entry MyAutotuner \
    --snapshot snapshot.json
```

## Documentation

- Language Reference - Complete syntax guide
- Tutorial - Step-by-step tutorial
- Examples - Real examples

## Building from Source

```bash
cd autotuner
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install
```

## Running Tests

```bash
make test
```

## Examples

See ==examples/== directory:

    - ==voltage_sweep/== - Voltage sweep measurement example

Run example:

```bash
make run-voltage-sweep
```

## License

MPL-2.0
