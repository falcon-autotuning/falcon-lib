# Falcon Autotuner Makefile
# Manages build configurations and testing

.PHONY: all configure build-debug build-release test clean install help

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

# Paths
VCPKG_ROOT ?= $(CURDIR)/../.vcpkg
VCPKG_TOOLCHAIN ?= $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
VCPKG_INSTALLED_DIR ?= $(CURDIR)/../vcpkg_installed

BUILD_DIR_DEBUG := build/debug
BUILD_DIR_RELEASE := build/release

all: build-release

help:
	@echo "Falcon Autotuner Build System"
	@echo "=============================="
	@echo ""
	@echo "Build targets:"
	@echo "  make build-debug    - Build debug version"
	@echo "  make build-release  - Build release version"
	@echo "  make configure      - Configure both builds"
	@echo "  make clean          - Clean build artifacts"
	@echo "  make install        - Install the library"
	@echo ""
	@echo "Test targets:"
	@echo "  make test           - Run all tests"
	@echo "  make test-debug     - Run debug tests"
	@echo "  make test-verbose   - Run tests with verbose output"
	@echo ""
	@echo "Example targets:"
	@echo "  make examples       - Build all examples"
	@echo "  make run-voltage-sweep - Run voltage sweep example"
	@echo ""
	@echo "Current configuration:"
	@echo "  Platform: $(PLATFORM)"
	@echo "  Generator: $(CMAKE_GENERATOR)"
	@echo "  Triplet: $(VCPKG_TRIPLET)"

configure-debug:
	@echo "Configuring debug build..."
	@mkdir -p $(BUILD_DIR_DEBUG)
	cd $(BUILD_DIR_DEBUG) && cmake ../.. \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_TOOLCHAIN_FILE=$(VCPKG_TOOLCHAIN) \
		-DVCPKG_INSTALLED_DIR=$(VCPKG_INSTALLED_DIR) \
		-DVCPKG_TARGET_TRIPLET=$(VCPKG_TRIPLET) \
		-DBUILD_TESTS=ON \
		-DBUILD_EXAMPLES=ON \
		-DBUILD_TOOLS=ON \
		-DUSE_CCACHE=ON \
		-G $(CMAKE_GENERATOR)
	@echo "✓ Debug build configured"

configure-release:
	@echo "Configuring release build..."
	@mkdir -p $(BUILD_DIR_RELEASE)
	cd $(BUILD_DIR_RELEASE) && cmake ../.. \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_TOOLCHAIN_FILE=$(VCPKG_TOOLCHAIN) \
		-DVCPKG_INSTALLED_DIR=$(VCPKG_INSTALLED_DIR) \
		-DVCPKG_TARGET_TRIPLET=$(VCPKG_TRIPLET) \
		-DBUILD_TESTS=ON \
		-DBUILD_EXAMPLES=ON \
		-DBUILD_TOOLS=ON \
		-DUSE_CCACHE=ON \
		-G $(CMAKE_GENERATOR)
	@echo "✓ Release build configured"

configure: configure-debug configure-release

build-debug: configure-debug
	@echo "Building debug..."
	cmake --build $(BUILD_DIR_DEBUG) --config Debug -j$(NPROC)
	@echo "✓ Debug build complete"

build-release: configure-release
	@echo "Building release..."
	cmake --build $(BUILD_DIR_RELEASE) --config Release -j$(NPROC)
	@echo "✓ Release build complete"

test: build-release
	@echo "Running tests..."
	cd $(BUILD_DIR_RELEASE) && ctest --output-on-failure -C Release
	@echo "✓ All tests passed"

test-debug: build-debug
	@echo "Running debug tests..."
	cd $(BUILD_DIR_DEBUG) && ctest --output-on-failure -C Debug
	@echo "✓ All tests passed"

test-verbose: build-release
	@echo "Running tests with verbose output..."
	cd $(BUILD_DIR_RELEASE) && ctest --verbose -C Release

examples: build-release
	@echo "✓ Examples built"

run-voltage-sweep: examples
	@echo "Running voltage sweep example..."
	cd $(BUILD_DIR_RELEASE)/examples/voltage_sweep && \
		../../falcon-runtime ./libvoltage_sweep_autotuner.so \
		--entry VoltageSweep \
		--snapshot ../../../examples/voltage_sweep/snapshot.json \
		--analyze

install: build-release
	@echo "Installing falcon-autotuner..."
	cmake --install $(BUILD_DIR_RELEASE)
	@echo "✓ Installation complete"

clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR_DEBUG) $(BUILD_DIR_RELEASE) build/
	@echo "✓ Clean complete"
