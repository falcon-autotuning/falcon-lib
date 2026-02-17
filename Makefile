# Falcon-lib Root Makefile
# Manages build configurations for all submodules

.PHONY: all deps help clean install-vcpkg-deps build-all test-all

VCPKG_ROOT ?= $(CURDIR)/.vcpkg
VCPKG_TOOLCHAIN ?= $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
    VCPKG_TRIPLET ?= x64-linux
endif
ifeq ($(OS),Windows_NT)
    VCPKG_TRIPLET ?= x64-windows
endif

all: build-all

deps:
	@if [ ! -d "$(VCPKG_ROOT)" ]; then \
		echo "Installing vcpkg to $(VCPKG_ROOT)..."; \
		git clone https://github.com/microsoft/vcpkg.git $(VCPKG_ROOT); \
		cd $(VCPKG_ROOT) && ./bootstrap-vcpkg.sh; \
	else \
		echo "vcpkg already installed at $(VCPKG_ROOT)"; \
		cd $(VCPKG_ROOT) && git pull; \
	fi
	@echo "✓ vcpkg ready"

install-vcpkg-deps: deps
	@echo "Installing vcpkg dependencies from vcpkg.json..."
	CC=clang CXX=clang++ MAKELEVEL=0 $(VCPKG_ROOT)/vcpkg install --triplet $(VCPKG_TRIPLET)
	@echo "✓ vcpkg dependencies installed"

build-all: install-vcpkg-deps
	@echo "Building all components..."
	$(MAKE) -C database build-release
	$(MAKE) -C database install
	$(MAKE) -C autotuner build-release
	$(MAKE) -C autotuner install
	$(MAKE) -C routine build-release
	$(MAKE) -C routine install
	@echo "✓ All components built"

test-all:
	@echo "Testing all components..."
	$(MAKE) -C database test
	$(MAKE) -C autotuner test
	@echo "✓ All tests passed"

install:
	@echo "Installing all components..."
	$(MAKE) -C database install
	$(MAKE) -C autotuner install
	@echo "✓ All components installed"

clean:
	@echo "Cleaning all components..."
	$(MAKE) -C database clean
	$(MAKE) -C autotuner clean
	rm -rf $(VCPKG_ROOT)
	rm -rf ./vcpkg_installed/
	@echo "✓ Clean complete"

help:
	@echo "Falcon Library Root Makefile"
	@echo "============================"
	@echo ""
	@echo "Setup targets:"
	@echo "  make deps               - Install or update vcpkg"
	@echo "  make install-vcpkg-deps - Install all dependencies"
	@echo ""
	@echo "Build targets:"
	@echo "  make build-all         - Build all components"
	@echo "  make test-all          - Test all components"
	@echo "  make install           - Install all components"
	@echo "  make clean             - Clean all builds"
	@echo ""
	@echo "Component-specific:"
	@echo "  make -C database <target>   - Run target in database/"
	@echo "  make -C autotuner <target>  - Run target in autotuner/"
	@echo ""
	@echo "Current configuration:"
	@echo "  VCPKG_ROOT: $(VCPKG_ROOT)"
	@echo "  VCPKG_TRIPLET: $(VCPKG_TRIPLET)"
