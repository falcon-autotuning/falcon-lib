# Falcon Autotuner Makefile
# Manages build configurations and testing using vcpkg

.PHONY: all configure build-debug build-release test clean install help check-vcpkg

# Detect OS
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PLATFORM := linux
    CMAKE_GENERATOR := Ninja
    VCPKG_TRIPLET ?= x64-linux-dynamic
    NPROC := $(shell nproc 2>/dev/null || echo 4)
endif
ifeq ($(OS),Windows_NT)
    PLATFORM := windows
    CMAKE_GENERATOR := "Visual Studio 17 2022"
    VCPKG_TRIPLET ?= x64-windows
    NPROC := 4
endif

# Paths (relative to autotuner/ directory)
VCPKG_ROOT ?= $(CURDIR)/../.vcpkg
VCPKG_TOOLCHAIN ?= $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
VCPKG_INSTALLED_DIR ?= $(CURDIR)/../vcpkg_installed

BUILD_DIR_DEBUG := build/debug
BUILD_DIR_RELEASE := build/release

# Default target
all: build-release

help:
	@echo "Falcon Autotuner Build System"
	@echo "=============================="
	@echo ""
	@echo "Build targets:"
	@echo "  make build-debug    - Build debug version"
	@echo "  make build-release  - Build release version"
	@echo "  make configure      - Configure both debug and release builds"
	@echo "  make clean          - Clean build artifacts"
	@echo "  make install        - Install the library"
	@echo ""
	@echo "Test targets:"
	@echo "  make test           - Run all tests (release)"
	@echo "  make test-debug     - Run tests (debug)"
	@echo "  make test-verbose   - Run tests with verbose output"
	@echo "  make test-unit      - Run only unit tests"
	@echo "  make test-integration - Run only integration tests"
	@echo ""
	@echo "Plugin targets:"
	@echo "  make plugins        - Build example plugins"
	@echo ""
	@echo "Documentation targets:"
	@echo "  make graph-viz      - Generate visualization examples"
	@echo ""
	@echo "Environment variables:"
	@echo "  VCPKG_ROOT          - Path to vcpkg root (default: ../.vcpkg)"
	@echo "  VCPKG_TRIPLET       - vcpkg triplet (default: x64-linux-dynamic)"
	@echo ""
	@echo "Current configuration:"
	@echo "  Platform: $(PLATFORM)"
	@echo "  Generator: $(CMAKE_GENERATOR)"
	@echo "  Triplet: $(VCPKG_TRIPLET)"
	@echo "  Vcpkg root: $(VCPKG_ROOT)"
	@echo "  Vcpkg toolchain: $(VCPKG_TOOLCHAIN)"
	@echo "  Vcpkg installed: $(VCPKG_INSTALLED_DIR)"

check-vcpkg:
	@echo "Checking vcpkg configuration..."
	@if [ ! -d "$(VCPKG_ROOT)" ]; then \
		echo "Error: vcpkg not found at $(VCPKG_ROOT)"; \
		echo "Run 'make deps' in the parent directory first"; \
		exit 1; \
	fi
	@if [ ! -f "$(VCPKG_TOOLCHAIN)" ]; then \
		echo "Error: vcpkg toolchain not found at $(VCPKG_TOOLCHAIN)"; \
		exit 1; \
	fi
	@if [ ! -d "$(VCPKG_INSTALLED_DIR)" ]; then \
		echo "Error: vcpkg_installed not found at $(VCPKG_INSTALLED_DIR)"; \
		echo "Run 'make install-vcpkg-deps' in the parent directory first"; \
		exit 1; \
	fi
	@echo "✓ vcpkg configuration OK"

configure-debug: check-vcpkg
	@echo "Configuring debug build..."
	@mkdir -p $(BUILD_DIR_DEBUG)
	cd $(BUILD_DIR_DEBUG) && cmake ../.. \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_TOOLCHAIN_FILE=$(VCPKG_TOOLCHAIN) \
		-DVCPKG_INSTALLED_DIR=$(VCPKG_INSTALLED_DIR) \
		-DVCPKG_TARGET_TRIPLET=$(VCPKG_TRIPLET) \
		-DBUILD_TESTS=ON \
		-DBUILD_EXAMPLES=ON \
		-DUSE_CCACHE=ON \
		-DENABLE_PCH=ON \
		-G $(CMAKE_GENERATOR)
	@echo "✓ Debug build configured"

configure-release: check-vcpkg
	@echo "Configuring release build..."
	@mkdir -p $(BUILD_DIR_RELEASE)
	cd $(BUILD_DIR_RELEASE) && cmake ../.. \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_TOOLCHAIN_FILE=$(VCPKG_TOOLCHAIN) \
		-DVCPKG_INSTALLED_DIR=$(VCPKG_INSTALLED_DIR) \
		-DVCPKG_TARGET_TRIPLET=$(VCPKG_TRIPLET) \
		-DBUILD_TESTS=ON \
		-DBUILD_EXAMPLES=ON \
		-DUSE_CCACHE=ON \
		-DENABLE_PCH=ON \
		-G $(CMAKE_GENERATOR)
	@echo "✓ Release build configured"

configure: configure-debug configure-release

build-debug: configure-debug
	@echo "Building debug..."
	cmake --build $(BUILD_DIR_DEBUG) --config Debug -j$(NPROC)
	@echo "✓ Debug build complete"
	@$(MAKE) clangd-helpers

build-release: configure-release
	@echo "Building release..."
	cmake --build $(BUILD_DIR_RELEASE) --config Release -j$(NPROC)
	@echo "✓ Release build complete"

test: build-release
	@echo "Running tests (release)..."
	cd $(BUILD_DIR_RELEASE) && ctest --output-on-failure -C Release
	@echo "✓ All tests passed"

test-debug: build-debug
	@echo "Running tests (debug)..."
	cd $(BUILD_DIR_DEBUG) && ctest --output-on-failure -C Debug
	@echo "✓ All tests passed"

test-verbose: build-release
	@echo "Running tests with verbose output..."
	cd $(BUILD_DIR_RELEASE) && ctest --verbose -C Release

test-unit: build-release
	@echo "Running unit tests..."
	cd $(BUILD_DIR_RELEASE) && ./falcon_autotuner_unit_tests

test-integration: build-release
	@echo "Running integration tests..."
	cd $(BUILD_DIR_RELEASE) && ./falcon_autotuner_integration_tests

plugins: build-release
	@echo "Building plugins..."
	cmake --build $(BUILD_DIR_RELEASE) --target example_plugin -j$(NPROC)
	@echo "✓ Plugins built"

graph-viz: build-release
	@echo "Generating graph visualizations..."
	@echo "Run test executables to generate DOT files"

install: build-release
	@echo "Installing falcon-autotuner..."
	cmake --install $(BUILD_DIR_RELEASE)
	@echo "✓ Installation complete"

clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR_DEBUG) $(BUILD_DIR_RELEASE) build/ compile_commands.json
	@echo "✓ Clean complete"

# Quick rebuild without reconfigure
rebuild-debug:
	@echo "Rebuilding debug..."
	cmake --build $(BUILD_DIR_DEBUG) --config Debug -j$(NPROC)

rebuild-release:
	@echo "Rebuilding release..."
	cmake --build $(BUILD_DIR_RELEASE) --config Release -j$(NPROC)

.PHONY: clangd-helpers
clangd-helpers:
	@if [ -f $(BUILD_DIR_DEBUG)/compile_commands.json ]; then \
		ln -sf $(BUILD_DIR_DEBUG)/compile_commands.json compile_commands.json; \
		echo "✓ clangd compile_commands.json symlinked to autotuner/ root"; \
	else \
		echo "No compile_commands.json found in debug build directory."; \
	fi
