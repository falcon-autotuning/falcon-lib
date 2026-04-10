# Falcon dsl Makefile
# Manages build configurations and testing

.PHONY: all configure build-debug build-release test clean install install-dsl install-lsp install-pm help

# Detect OS
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PLATFORM := linux
    CMAKE_GENERATOR := Ninja
    VCPKG_TRIPLET ?= x64-linux-dynamic
    NPROC := $(shell nproc 2>/dev/null || echo 4)
    SUDO ?= sudo
		export CC=clang
		export CXX=clang++
endif
ifeq ($(OS),Windows_NT)
    PLATFORM := windows
    CMAKE_GENERATOR := "Visual Studio 17 2022"
    VCPKG_TRIPLET ?= x64-windows
    NPROC := 4
SUDO := 
endif

# Paths
VCPKG_ROOT ?= $(CURDIR)/vcpkg
VCPKG_TOOLCHAIN ?= $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
VCPKG_INSTALLED_DIR ?= $(CURDIR)/vcpkg_installed
NUGET_FEED ?= https://pkgs.dev.azure.com/falcon-autotuning/_packaging/falcon-autotuning/nuget/v3/index.json
VCPKG_BINARY_SOURCES ?= nuget,$(NUGET_FEED),readwrite

BUILD_DIR_DEBUG := build/debug
BUILD_DIR_RELEASE := build/release

INSTALL_PREFIX    ?= /opt/falcon
INSTALL_LIBDIR    := $(INSTALL_PREFIX)/lib
INSTALL_INCLUDEDIR := $(INSTALL_PREFIX)/include
INSTALL_CMAKEDIR  := $(INSTALL_LIBDIR)/cmake/falcon-dsl

all: build-release

help:
	@echo "Falcon DSL Build System"
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
	@echo "Prerequisites:"
	@echo "  falcon-typing must be installed first (cd ../typing && make install)"
	@echo ""
	@echo "Current configuration:"
	@echo "  Platform: $(PLATFORM)"
	@echo "  Generator: $(CMAKE_GENERATOR)"
	@echo "  Triplet: $(VCPKG_TRIPLET)"

.PHONY: vcpkg-bootstrap
vcpkg-bootstrap:
	@if [ ! -d "$(VCPKG_ROOT)" ]; then \
		echo "Cloning vcpkg..."; \
		git clone https://github.com/microsoft/vcpkg.git $(VCPKG_ROOT); \
	fi
	@if [ ! -f "$(VCPKG_ROOT)/vcpkg" ]; then \
		echo "Bootstrapping vcpkg..."; \
		cd $(VCPKG_ROOT) && ./bootstrap-vcpkg.sh; \
	fi

.PHONY: vcpkg-install-deps
vcpkg-install-deps:
	@echo "Installing vcpkg dependencies" 
	@API_KEY=$$(if [ -f .nuget_api_key ]; then cat .nuget_api_key; else echo $$NUGET_API_KEY; fi); \
	if [ ! -z "$$API_KEY" ] && [ ! -z "$(NUGET_FEED)" ]; then \
		echo "Generating temporary nuget.config for binary caching..."; \
		printf '<?xml version="1.0" encoding="utf-8"?>\n<configuration>\n  <config>\n    <add key="defaultPushSource" value="AzureDevOps" />\n  </config>\n  <packageSources>\n    <add key="AzureDevOps" value="%s" />\n  </packageSources>\n  <packageSourceCredentials>\n    <AzureDevOps>\n      <add key="Username" value="az" />\n      <add key="ClearTextPassword" value="%s" />\n    </AzureDevOps>\n  </packageSourceCredentials>\n</configuration>\n' "$(NUGET_FEED)" "$$API_KEY" > $(VCPKG_ROOT)/nuget.config; \
		export VCPKG_BINARY_SOURCES="clear;nugetconfig,$(VCPKG_ROOT)/nuget.config,readwrite"; \
	elif [ ! -z "$(VCPKG_BINARY_SOURCES)" ]; then \
		echo "Using existing VCPKG_BINARY_SOURCES: $(VCPKG_BINARY_SOURCES)"; \
		export VCPKG_BINARY_SOURCES="$(VCPKG_BINARY_SOURCES)"; \
	fi && \
	CC=clang CXX=clang++ VCPKG_FEATURE_FLAGS=binarycaching \
		$(VCPKG_ROOT)/vcpkg install \
		--triplet="$(VCPKG_TRIPLET)" \
		--overlay-ports=./ports \
		--debug

check-vcpkg: vcpkg-bootstrap  vcpkg-install-deps
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
	@echo "✓ vcpkg configuration OK"

configure-debug: check-vcpkg
	@echo "Configuring debug build..."
	@mkdir -p $(BUILD_DIR_DEBUG)
	cd $(BUILD_DIR_DEBUG) && cmake ../.. \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_TOOLCHAIN_FILE=$(VCPKG_TOOLCHAIN) \
		-DVCPKG_INSTALLED_DIR=$(VCPKG_INSTALLED_DIR) \
		-DVCPKG_TARGET_TRIPLET=$(VCPKG_TRIPLET) \
		-Dlsp_DIR=/opt/falcon/lib/cmake/lsp \
		-DBUILD_TESTS=ON \
		-DBUILD_TOOLS=ON \
		-DUSE_CCACHE=ON \
		-DENABLE_PCH=ON \
		-DCMAKE_C_COMPILER=clang \
		-DCMAKE_CXX_COMPILER=clang++ \
		-DVCPKG_BINARY_SOURCES="$(VCPKG_BINARY_SOURCES)" \
		-DVCPKG_OVERLAY_PORTS=../../ports \
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
		-Dlsp_DIR=/opt/falcon/lib/cmake/lsp \
		-DBUILD_TESTS=ON \
		-DBUILD_TOOLS=ON \
		-DUSE_CCACHE=ON \
		-DENABLE_PCH=ON \
		-DCMAKE_C_COMPILER=clang \
		-DCMAKE_CXX_COMPILER=clang++ \
		-DVCPKG_BINARY_SOURCES="$(VCPKG_BINARY_SOURCES)" \
		-DVCPKG_OVERLAY_PORTS=../../ports \
		-G $(CMAKE_GENERATOR)
	@echo "✓ Release build configured"

configure: configure-debug configure-release

build-debug: configure-debug
	@echo "Building debug..."
	ninja -C $(BUILD_DIR_DEBUG) -j$(NPROC)
	@echo "✓ Debug build complete"
	@$(MAKE) clangd-helpers

build-release: configure-release
	@echo "Building release..."
	ninja -C $(BUILD_DIR_RELEASE) -j$(NPROC)
	@echo "✓ Release build complete"


# ── Install ────────────────────────────────────────────────────────────────────
install: install-dsl install-lsp install-pm
	@echo "✓ Full DSL suite installed to $(INSTALL_PREFIX)"

install-dsl: build-release
	@echo "Installing falcon-dsl to $(INSTALL_PREFIX)..."
	$(SUDO) cmake --install $(BUILD_DIR_RELEASE) --prefix $(INSTALL_PREFIX)
	$(SUDO) cp -v $(BUILD_DIR_RELEASE)/falcon-run $(INSTALL_LIBDIR)/falcon-run
	$(SUDO) cp -v $(BUILD_DIR_RELEASE)/falcon-test $(INSTALL_LIBDIR)/falcon-test
	$(SUDO) cp -v $(BUILD_DIR_RELEASE)/package-manager/falcon-pm $(INSTALL_LIBDIR)/falcon-pm
	$(SUDO) cp -v $(BUILD_DIR_RELEASE)/package-manager/libfalcon-pm.a $(INSTALL_LIBDIR)/libfalcon-pm.a
	@echo "✓ falcon-dsl installed"
	@echo "  Library : $(INSTALL_LIBDIR)/libfalcon-dsl.so"
	@echo "  Headers : $(INSTALL_INCLUDEDIR)/falcon-dsl/"
	@echo "  CMake   : $(INSTALL_CMAKEDIR)/"
	@echo "  CLI     : $(INSTALL_LIBDIR)/falcon-run"
	@echo "  TEST    : $(INSTALL_LIBDIR)/falcon-test"

install-pm: build-release
	@echo "Installing falcon-pm (package manager) to $(INSTALL_PREFIX)..."
	$(SUDO) mkdir -p $(INSTALL_LIBDIR)
	$(SUDO) mkdir -p $(INSTALL_INCLUDEDIR)/falcon-pm
	$(SUDO) cp -v $(BUILD_DIR_RELEASE)/package-manager/libfalcon-pm.a $(INSTALL_LIBDIR)/ 2>/dev/null || true
	$(SUDO) find $(BUILD_DIR_RELEASE)/package-manager -name "falcon-pm" -type f \
		-exec cp -v {} $(INSTALL_PREFIX)/lib/falcon-pm \; 2>/dev/null || true
	@if [ -d lsp/package-manager/include ]; then \
		$(SUDO) cp -r package-manager/include/* $(INSTALL_INCLUDEDIR)/falcon-pm/; \
	fi
	@echo "✓ falcon-pm installed"

# ── Uninstall ─────────────────────────────────────────────────────────────────
uninstall:
	@echo "Uninstalling falcon-dsl from $(INSTALL_PREFIX)..."
	$(SUDO) rm -f  $(INSTALL_LIBDIR)/libfalcon-dsl.so
	$(SUDO) rm -f  $(INSTALL_LIBDIR)/libfalcon-dsl.so.1
	$(SUDO) rm -f  $(INSTALL_LIBDIR)/libfalcon-dsl.so.1.0.0
	@if [ -d "$(INSTALL_INCLUDEDIR)/falcon-dsl" ]; then $(SUDO) rm -r "$(INSTALL_INCLUDEDIR)/falcon-dsl"; fi
	@if [ -d "$(INSTALL_INCLUDEDIR)/falcon-pm" ]; then $(SUDO) rm -r "$(INSTALL_INCLUDEDIR)/falcon-pm"; fi
	@if [ -d "$(INSTALL_INCLUDEDIR)/falcon-lsp" ]; then $(SUDO) rm -r "$(INSTALL_INCLUDEDIR)/falcon-lsp"; fi
	@if [ -d "$(INSTALL_CMAKEDIR)" ]; then $(SUDO) rm -r "$(INSTALL_CMAKEDIR)"; fi
	$(SUDO) rm -f  $(INSTALL_LIBDIR)/falcon-lsp
	$(SUDO) rm -f  $(INSTALL_LIBDIR)/falcon-pm
	$(SUDO) rm -f  $(INSTALL_LIBDIR)/libfalcon-atc-core
	$(SUDO) rm -f  $(INSTALL_LIBDIR)/libfalcon-pm
	$(SUDO) rm -f  $(INSTALL_LIBDIR)/falcon-run
	$(SUDO) rm -f  $(INSTALL_LIBDIR)/falcon-test
	@echo "✓ Uninstall complete"

install-lsp: build-release
	@echo "Installing lsp library to $(INSTALL_PREFIX)..."
	$(SUDO) mkdir -p $(INSTALL_LIBDIR)
	$(SUDO) mkdir -p $(INSTALL_INCLUDEDIR)/falcon-lsp
	$(SUDO) cp -v $(BUILD_DIR_RELEASE)/lsp/libfalcon-lsp.a $(INSTALL_LIBDIR)/
	$(SUDO) cp -v $(BUILD_DIR_RELEASE)/lsp/falcon-lsp $(INSTALL_LIBDIR)/
	@if [ -d $(BUILD_DIR_RELEASE)/lsp/include ]; then \
		$(SUDO) cp -r $(BUILD_DIR_RELEASE)/lsp/include/* $(INSTALL_INCLUDEDIR)/lsp/; \
	fi
	@echo "✓ lsp library installed"

clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR_DEBUG) $(BUILD_DIR_RELEASE) build/ compile_commands.json vcpkg_installed/
	@$(MAKE) docker-clean
	@echo "✓ Clean complete"

.PHONY: clangd-helpers
clangd-helpers:
	@if [ -f $(BUILD_DIR_DEBUG)/compile_commands.json ]; then \
		ln -sf $(BUILD_DIR_DEBUG)/compile_commands.json compile_commands.json; \
		echo "✓ clangd compile_commands.json symlinked to dsls/ root"; \
	else \
		echo "No compile_commands.json found in debug build directory."; \
	fi

test: build-release
	@cd $(BUILD_DIR_RELEASE) && \
		ctest --verbose -C Release
	@echo "✓ All tests passed"

test-debug: build-debug
	@cd $(BUILD_DIR_DEBUG)/package-manager && \
		./falcon-pm-tests $(if $(GTEST_FILTER),--gtest_filter="$(GTEST_FILTER)",)
	@echo "Running dsl tests..."
	@cd $(BUILD_DIR_DEBUG)/tests && \
		LD_LIBRARY_PATH=$(CURDIR)/$(BUILD_DIR_DEBUG):$(VCPKG_INSTALLED_DIR)/$(VCPKG_TRIPLET)/lib:$(INSTALL_PREFIX)/lib:$$LD_LIBRARY_PATH \
		./falcon_dsl_integration_tests $(if $(GTEST_FILTER),--gtest_filter="$(GTEST_FILTER)",)
	@echo "Running lsp tests..."
	@cd $(BUILD_DIR_DEBUG)/lsp/tests && \
		./falcon_lsp_unit_tests $(if $(GTEST_FILTER),--gtest_filter="$(GTEST_FILTER)",) && \
		./falcon_lsp_integration_tests $(if $(GTEST_FILTER),--gtest_filter="$(GTEST_FILTER)",)
	@echo "✓ All tests passed"

