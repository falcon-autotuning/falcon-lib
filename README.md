# Falcon Lib

**Falcon Lib** is a comprehensive C++ and DSL-based framework for quantum device autotuning. It bridges measurement requests from the Falcon core system with physical hardware instruments, providing a complete suite of libraries, tools, and examples for building robust quantum device control workflows.

## What's Inside

Falcon Lib is organized into several key components:

- **DSL (Domain-Specific Language)**: A high-level language for defining quantum device autotuning state machines
- **Database**: C++ PostgreSQL backend for storing device characteristics
- **Communications (comms)**: NATS-based messaging infrastructure for device control
- **QArray Device Module**: Specialized support for quantum dot array devices
- **Core Libraries**: Collections, math utilities, physics models, and device structures
- **Typing System**: Runtime type system and FFI (Foreign Function Interface) support

## Quick Start

Currently this application only works on Linux. This may work on windows with WSL2.
Windows support is currently in the works for better instrument support.

See the [documentation](https://falcon-autotuning.github.io/falcon-lib/).

### For Users: Run the Demo

```bash
cd demos/qarray-charge-tuning
make docker-up
falcon-test ./tests/run_tests.fal --log-level info
```

See [demos](https://github.com/falcon-autotuning/falcon-lib/demos/qarray-charge-tuning/README.md) for more details.

### For Developers: Build from Source

Prerequisites

- CMake >= 3.20
- C++20 compiler
- PostgreSQL (for database support)
- Bison  >= 3.8
- Docker (optional, for running services)

Build Steps

```bash
# Install dependencies
make deps

# Build all components
make build

# Run tests (requires Docker or external PostgreSQL/NATS)
make test

# Or without Docker (services must be running)
export TEST_DATABASE_URL="postgresql://falcon_test:falcon_test_password@127.0.0.1:5432/falcon_test"
export TEST_NATS_URL="nats://localhost:4222"
make test-local
```

Component-Specific Build

Each major component has its own build instructions:

- DSL: **cd dsl && make install**
- Database: **cd database && make install**
- Comms: **cd comms && make install**
- Typing: **cd typing && make install**
- qarrayDevice: **cd typing && make install**

## Main Features

✨ Quantum Device Autotuning: Define complex control workflows as declarative state machines

🔌 Hardware Integration: FFI support for binding C++ measurement functions to .fal autotuners

📊 Device Database: PostgreSQL-backed storage for device characteristics and global tuning parameters

🌐 NATS Messaging: Distributed communication for multi-device systems

🧪 Built-in Testing: Test runner with setup/teardown fixtures and detailed diagnostics

📚 Language Server: IDE support via LSP for .fal files (Neovim)

## Next Steps

- Get Started: Read [dsl/README.md](/dsl/README.md) for the introduction to the language
- Try the Demo: Run the QArray charge tuning [demo](https://github.com/falcon-autotuning/falcon-lib/tree/main/demos/qarray-charge-tuning)
- Develop: Check individual component READMEs for build and API details
- Online Docs: Visit our Falcon Lib website for interactive documentation

## Contributing

We welcome contributions! Please refer to our contributing guidelines in the documentation.

## License

MPL-2.0
