
# Falcon DSL Overview

The Falcon DSL (`.fal`) is a domain-specific language for defining state machines used in quantum device autotuning.

## Key Features

- **Typed Inputs & Outputs** — Explicit type signatures for autotuners and routines
- **State Machine Control** — Sequential execution through named states with explicit transitions
- **Composability** — Import routines and structs across modules
- **Generic Types** — Parameterized structs for reusable data structures
- **FFI Integration** — Direct C++ hardware binding via `ffimport`

## Quick Start

1. **Learn the Language**: Read the [Language Reference](LANGUAGE_REFERENCE.md)
2. **Follow a Tutorial**: Build your first autotuner with the [Tutorial](TUTORIAL.md)
3. **Run Autotuners**: Use the [CLI Reference](CLI.md) for `falcon-run`
4. **Manage Dependencies**: Use the [Package Manager](PACKAGE_MANAGER.md)
5. **Set Up Your Editor**: Install [Language Server Protocol](LSP.md) support

## Components

### Core Tools

- **`falcon-run`** — Execute autotuners from the command line
- **`falcon-pm`** — Manage imports and dependencies
- **`falcon-lsp`** — Language server for IDE integration

### Syntax Highlighting & IDE Support

- **TreeSitter Grammar** ([treesitter/](../treesitter/README.md)) — Syntax parsing and highlighting
- **Language Server** ([lsp/](../lsp/README.md)) — IDE features: diagnostics, completion, go-to-definition

## File Structure

A typical project layout:
