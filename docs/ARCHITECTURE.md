
# Falcon Lib Architecture

Falcon Lib is a comprehensive quantum device autotuning framework with four main components:

## Components

### 1. **DSL & Runtime** (`dsl/`)

The domain-specific language and execution engine for defining state machine autotuners.

- **Compiler** — Parses `.fal` files and generates bytecode
- **AutotunerEngine** — Executes autotuners at runtime
- **Package Manager** (`falcon-pm`) — Resolves imports and dependencies

**Entry Point**: [DSL Overview](dsl/docs/OVERVIEW.md)

### 2. **Database** (`database/`)

Persistent storage for calibration results and autotuner execution history.

- State snapshots
- Parameter sweeps and measurements
- Execution traces and debugging

**Entry Point**: [Database README](database/README.md)

### 3. **FFI & Type System** (`typing/`)

C ABI wrapper layer for binding C++ hardware code to the DSL.

- Type marshalling
- Memory management helpers
- Standard library bindings

**Entry Point**: [FFI Reference](typing/README.md)

### 4. **Hardware Integration** (`qarrayDevice/`, `libs/`)

#### qArray Device (`qarrayDevice/`)

Quantum array device control and measurement.

**Entry Point**: [qArray Device](qarrayDevice/README.md)

#### Library Bindings (`libs/`)

Standard library of falcon DSL language bindings.

**Entry Point**: [Library Bindings](libs/README.md)

## Information Flow
