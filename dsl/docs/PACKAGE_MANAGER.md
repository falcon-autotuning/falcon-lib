# Falcon Package Manager (`falcon-pm`)

The Falcon package manager resolves `import` statements in `.fal` files and manages project dependencies declared in a `falcon.yml` manifest.

---

## Overview

When `AutotunerEngine::load_fal_file()` encounters an `import "..."` statement it delegates path resolution to `falcon::pm::PackageManager`. The package manager:

1. Resolves relative paths against the importing file
2. Checks the project's `falcon.yml` for named packages
3. Falls back to system package paths

---

## `falcon.yml` Manifest

Place a `falcon.yml` in your project root (same directory as your top-level `.fal` file):

```yaml
name: my-quantum-experiment
version: 1.2.0

dependencies:
  shared-types:    "../shared/types"
  voltage-routines: "/opt/falcon/packages/voltage"
  calibration:     "github.com/myorg/calibration@v2.1.0"
```

### Fields

| Field | Required | Description |
|-------|----------|-------------|
| `name` | Yes | Project name |
| `version` | Yes | Semantic version |
| `dependencies` | No | Named package references |

---

## Import resolution order

Given `import "some/path.fal"`:

1. **Relative** — `<importing_file_dir>/some/path.fal`
2. **Project root** — `<falcon.yml_dir>/some/path.fal`
3. **Dependency alias** — if `some/path` matches a dependency key, use that location
4. **System path** — `/opt/falcon/packages/some/path.fal`

---

## CLI (`falcon-pm`)

```bash
# Show resolved import graph for a file
falcon-pm resolve my_autotuner.fal

# Install/fetch packages declared in falcon.yml
falcon-pm install

# Check what version of a package is resolved
falcon-pm info voltage-routines
```

---

## Circular import detection

`AutotunerEngine` tracks loaded absolute paths and will refuse to load the same file twice, preventing infinite import loops. A warning is emitted if a cycle is detected.

---

## Package structure

A Falcon package is a directory containing:

```
my-package/
  falcon.yml          # package manifest
  types.fal           # exported autotuners/structs/routines
  routines.fal
  README.md
```

Other `.fal` files in the package are internal; only files explicitly imported by the entry file are available to consumers.

---

## Programmatic API

```cpp
#include <falcon-pm/PackageManager.hpp>

falcon::pm::PackageManager pm("/path/to/my_autotuner.fal");

std::vector<std::string> raw_imports = { "shared/types.fal" };
auto resolved = pm.resolve_imports("/path/to/my_autotuner.fal", raw_imports);

for (const auto& imp : resolved) {
    std::cout << imp.module_name    << " -> "
              << imp.absolute_path  << "\n";
}
```
