# Falcon Package Manager (`falcon-pm`)

The Falcon Package Manager (`falcon-pm`) handles dependency resolution, secure remote package distribution, and FFI (Foreign Function Interface) binary compilation for the Falcon DSL.

It guarantees **Environment Parity**: a package running locally in development behaves exactly the same as when it is downloaded remotely by an end user.

---

## Overview

When the `AutotunerEngine` encounters an `import "..."` statement, it delegates resolution to `falcon-pm`. The package manager:

1. Resolves relative paths against the importing file.
2. Checks the project's `.falcon/cache/` for downloaded external packages.
3. Automatically triggers local C++ compilation for missing FFI binaries (`.so` files) if raw `.cpp` files are present.
4. Verifies cryptographic hashes (SHA-256) of downloaded `.so` files to prevent supply-chain attacks.

---

## `falcon.yml` Manifest

Every Falcon package (including your root project) should have a `falcon.yml` file. This replaces ad-hoc environment paths with a declarative, secure manifest.

```yaml
name: "calibration-tools"
version: "1.2.0"
description: "Quantum experiment calibration routines"
maintainer: "Falcon Team"
license: "MPL-2.0"
github: "falcon-autotuning/calibration-tools"

# Secure FFI map: target binary -> expected SHA-256 hash
ffi:
  "calibration-wrapper.so": "sha256:d8d5da6d51c4e0504e5bec31965d9d635ccbd43e1ac10ae07f"

# Array of dependencies
dependencies:
  - name: "shared-types"
    version: "*"
    local_path: "../shared-types"
    
  - name: "testing"
    version: "1.0.0"
    github: "falcon-autotuning/testing"
```

### Fields

| Field | Required | Description |
|-------|----------|-------------|
| `name` | Yes | Project/Package name |
| `version` | Yes | Semantic version (e.g., `1.0.0`) |
| `description` | No | Short summary of the package |
| `maintainer` | No | Author or organization |
| `license` | No | Open source license (e.g., `MPL-2.0`) |
| `github` | No | Canonical GitHub repository URL for the package |
| `ffi` | No | Map of `.so` filenames to their secure `sha256:` hashes |
| `dependencies` | No | Array of dependencies defining `name`, `version`, and source (`github` or `local_path`) |

---

## CLI Reference (`falcon-pm`)

The `falcon-pm` executable manages your local workspace and remote dependencies.

```bash
# Initialize a new project (creates falcon.yml and .falcon/cache/)
falcon-pm --init . my-project

# Install a remote GitHub package (Downloads the GitHub Release .tar.gz)
falcon-pm --install github.com/falcon-autotuning/testing

# Install a local development package
falcon-pm --install ../shared-types

# List all currently installed packages in the cache
falcon-pm --list

# Remove a package
falcon-pm --remove testing

# Compile local C++ wrappers into .so files and update falcon.yml hashes
falcon-pm --build
```

---

## Distributing Packages & GitHub Releases

To distribute a Falcon package that includes C++ bindings, you must utilize GitHub Releases so users receive pre-compiled shared objects (`.so`) rather than forcing them to compile raw `.cpp` source files locally.

### How it works

1. When a user runs `falcon-pm --install github.com/org/repo`, the package manager queries GitHub for the **latest Release** and attempts to download `repo.tar.gz`.
2. It extracts the release. If a pre-built `.so` file is present, it computes its SHA-256 hash.
3. If the hash exactly matches the hash recorded in `falcon.yml`, the package is installed securely.
4. *Fallback:* If no GitHub release is found (or no `repo.tar.gz` exists), it falls back to downloading the raw source code archive.

### Preparing a Release

1. Develop your package and `.cpp` wrappers locally.
2. Run `falcon-pm --build`. This generates the `.so` and automatically injects its `sha256` hash into your `falcon.yml`.
3. Compress the package: `tar -czvf repo-name.tar.gz falcon.yml my-file.fal wrapper.so`.
4. Upload `repo-name.tar.gz` as a binary asset to a new GitHub Release.

---

## Package Cache Structure

All downloaded dependencies are kept strictly local to your project inside `.falcon/cache/`. The structure is entirely flat, organized by repository name.

```text
my-project/
├── falcon.yml
├── main.fal
└── .falcon/
    └── cache/
        ├── testing/                 <-- Downloaded GitHub Release
        │   ├── falcon.yml
        │   ├── testing.fal
        │   └── testing-wrapper.so
        │
        └── shared-types/            <-- Local Path Dependency
            ├── falcon.yml
            └── types.fal
```

This guarantees that external packages execute in a self-contained, predictable environment.

---

## Programmatic API

The package manager can be embedded directly into C++ applications (like the `AutotunerEngine`).

```cpp
#include <falcon-pm/PackageManager.hpp>

// Initialize pointing at the directory of your top-level script
falcon::pm::PackageManager pm("/path/to/project/main.fal");

// Manually trigger a build of local FFI files
pm.build(pm.project_root());

// Resolve import statements
std::vector<std::string> raw_imports = { "github.com/falcon-autotuning/testing" };
auto resolved = pm.resolve_imports("/path/to/project/main.fal", raw_imports);

for (const auto& imp : resolved) {
    std::cout << imp.module_name    << " -> "
              << imp.absolute_path  << "\n";
}
```
