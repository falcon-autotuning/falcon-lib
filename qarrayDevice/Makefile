# Falcon qarrayDevice Makefile
# Wraps cmake + vcpkg, following the same pattern as database/ and comms/

.PHONY: all configure build-debug build-release test clean install uninstall \
        help check-vcpkg install-python-deps clangd-helpers rebuild-debug rebuild-release

# ── OS detection ──────────────────────────────────────────────────────────────
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PLATFORM        := linux
    CMAKE_GENERATOR := Ninja
    VCPKG_TRIPLET   ?= x64-linux-dynamic
    NPROC           := $(shell nproc 2>/dev/null || echo 4)
    SUDO            := sudo
endif
ifeq ($(OS),Windows_NT)
    PLATFORM        := windows
    CMAKE_GENERATOR := "Visual Studio 17 2022"
    VCPKG_TRIPLET   ?= x64-windows
    NPROC           := 4
    SUDO            :=
endif

# ── Paths ─────────────────────────────────────────────────────────────────────
VCPKG_ROOT          ?= $(CURDIR)/../.vcpkg
VCPKG_TOOLCHAIN     ?= $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
VCPKG_INSTALLED_DIR ?= $(CURDIR)/../vcpkg_installed

BUILD_DIR_DEBUG   := build/debug
BUILD_DIR_RELEASE := build/release

INSTALL_PREFIX    ?= /opt/falcon
INSTALL_LIBDIR    := $(INSTALL_PREFIX)/lib
INSTALL_INCLUDEDIR := $(INSTALL_PREFIX)/include

# Python interpreter used for installing Python deps
PYTHON ?= python3

# ── Targets ────────────────────────────────────────────────────────────────���──
all: build-release

help:
	@echo "Falcon qarrayDevice Build System"
	@echo "=================================="
	@echo ""
	@echo "Build targets:"
	@echo "  make build-debug          - Build debug version"
	@echo "  make build-release        - Build release version"
	@echo "  make configure            - Configure both debug and release"
	@echo "  make clean                - Clean build artifacts"
	@echo "  make install              - Install the library to INSTALL_PREFIX"
	@echo ""
	@echo "Python targets:"
	@echo "  make install-python-deps  - Install Python dependencies via pip"
	@echo ""
	@echo "Test targets:"
	@echo "  make test                 - Run release tests"
	@echo "  make test-debug           - Run debug tests"
	@echo ""
	@echo "Environment variables:"
	@echo "  VCPKG_ROOT          - Path to vcpkg root    (default: ../.vcpkg)"
	@echo "  VCPKG_TRIPLET       - vcpkg triplet         (default: x64-linux-dynamic)"
	@echo "  INSTALL_PREFIX      - Install prefix        (default: /opt/falcon)"
	@echo "  PYTHON              - Python interpreter    (default: python3)"
	@echo "  FALCON_QARRAY_PYTHON_PATH - Runtime override for Python module dir"
	@echo ""
	@echo "Current configuration:"
	@echo "  Platform:  $(PLATFORM)"
	@echo "  Generator: $(CMAKE_GENERATOR)"
	@echo "  Triplet:   $(VCPKG_TRIPLET)"

# ── vcpkg check ───────────────────────────────────────────────────────────────
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

# ── Python dependencies ───────────────────────────────────────────────────────
# qarray-rust-core only has prebuilt wheels for Python <=3.12.
# Using Python 3.13+ will trigger a Rust source build which fails under CMake 4.x.
# We pin to 3.12 here. Override with: make install-python-deps PYTHON_VERSION=3.11
PYTHON_VERSION ?= 3.12

install-python-deps:
	@echo "Installing Python dependencies with uv (Python $(PYTHON_VERSION))..."
	@echo "Note: Python $(PYTHON_VERSION) required — qarray wheels not available for 3.13+"
	uv python pin $(PYTHON_VERSION)
	uv venv
	uv pip install -r requirements.txt
	@echo "✓ Python dependencies installed"
	@echo ""
	@echo "To activate: source .venv/bin/activate"
	@echo "To verify:   uv run python -c \"import qarray; print('qarray OK')\""

# ── CMake configure ───────────────────────────────────────────────────────────
configure-debug: check-vcpkg
	@echo "Configuring debug build..."
	@mkdir -p $(BUILD_DIR_DEBUG)
	cd $(BUILD_DIR_DEBUG) && cmake ../.. \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_TOOLCHAIN_FILE=$(VCPKG_TOOLCHAIN) \
		-DVCPKG_INSTALLED_DIR=$(VCPKG_INSTALLED_DIR) \
		-DVCPKG_TARGET_TRIPLET=$(VCPKG_TRIPLET) \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) \
		-DBUILD_TESTS=ON \
		-DUSE_CCACHE=ON \
		-DENABLE_PCH=ON \
		-DCMAKE_C_COMPILER=clang \
		-DCMAKE_CXX_COMPILER=clang++ \
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
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) \
		-DBUILD_TESTS=ON \
		-DUSE_CCACHE=ON \
		-DENABLE_PCH=ON \
		-DCMAKE_C_COMPILER=clang \
		-DCMAKE_CXX_COMPILER=clang++ \
		-G $(CMAKE_GENERATOR)
	@echo "✓ Release build configured"

configure: configure-debug configure-release

# ── Build ─────────────────────────────────────────────────────────────────────
build-debug: configure-debug
	@echo "Building debug..."
	ninja -C $(BUILD_DIR_DEBUG) -j$(NPROC)
	@echo "✓ Debug build complete"
	@$(MAKE) clangd-helpers

build-release: configure-release
	@echo "Building release..."
	ninja -C $(BUILD_DIR_RELEASE) -j$(NPROC)
	@echo "✓ Release build complete"

# ── Test ──────────────────────────────────────────────────────────────────────
test: build-release
	@echo "Running release tests..."
	cd $(BUILD_DIR_RELEASE) && ctest --output-on-failure -C Release
	@echo "✓ Tests passed"

test-debug: build-debug
	@echo "Running debug tests..."
	. .venv/bin/activate && cd $(BUILD_DIR_DEBUG) && ctest --output-on-failure -C Debug
	@echo "✓ Tests passed"

# ── Install ───────────────────────────────────────────────────────────────────
install: build-release
	@echo "Installing falcon-qarrayDevice to $(INSTALL_PREFIX)..."
	$(SUDO) cmake --install $(BUILD_DIR_RELEASE) --prefix $(INSTALL_PREFIX)
	@echo "Copying vcpkg headers and libraries..."
	$(SUDO) cp -r $(VCPKG_INSTALLED_DIR)/$(VCPKG_TRIPLET)/include/* $(INSTALL_INCLUDEDIR)/
	$(SUDO) cp -r $(VCPKG_INSTALLED_DIR)/$(VCPKG_TRIPLET)/lib/*     $(INSTALL_LIBDIR)/
	$(SUDO) cp -r $(VCPKG_INSTALLED_DIR)/$(VCPKG_TRIPLET)/bin/*     $(INSTALL_LIBDIR)/ || true
ifeq ($(UNAME_S),Linux)
	$(SUDO) ldconfig
endif
	@echo "✓ Installation complete"
	@echo ""
	@echo "NOTE: Python dependencies must also be installed:"
	@echo "  make install-python-deps"
	@echo "  (or: $(PYTHON) -m pip install -r requirements.txt)"

# ── Uninstall ─────────────────────────────────────────────────────────────────
uninstall:
	@echo "Uninstalling falcon-qarrayDevice from $(INSTALL_PREFIX)..."
	$(SUDO) rm -rf \
		$(INSTALL_LIBDIR)/libfalcon-qarrayDevice* \
		$(INSTALL_INCLUDEDIR)/qarrayDevice \
		$(INSTALL_PREFIX)/share/qarrayDevice \
		$(INSTALL_PREFIX)/lib/cmake/falcon-qarrayDevice || true
	@echo "✓ Uninstall complete"

# ── Clean ─────────────────────────────────────────────────────────────────────
clean: uninstall
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR_DEBUG) $(BUILD_DIR_RELEASE) build/ compile_commands.json
	@echo "✓ Clean complete"

# ── Quick rebuild (no reconfigure) ───────────────────────────────────────────
rebuild-debug:
	cmake --build $(BUILD_DIR_DEBUG)   --config Debug   -j$(NPROC)

rebuild-release:
	cmake --build $(BUILD_DIR_RELEASE) --config Release -j$(NPROC)

# ── clangd helpers ────────────────────────────────────────────────────────────
.PHONY: clangd-helpers
clangd-helpers:
	@if [ -f $(BUILD_DIR_DEBUG)/compile_commands.json ]; then \
		ln -sf $(BUILD_DIR_DEBUG)/compile_commands.json compile_commands.json; \
		echo "✓ clangd compile_commands.json symlinked to qarrayDevice/ root"; \
	else \
		echo "No compile_commands.json found in debug build directory."; \
	fi
