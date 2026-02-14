# Falcon-lib  Makefile
# Manages build configurations and testing using vcpkg
.PHONY: all deps help clean

VCPKG_ROOT ?= $(CURDIR)/.vcpkg
VCPKG_TOOLCHAIN ?= $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    VCPKG_TRIPLET ?= x64-linux-dynamic
endif
ifeq ($(OS),Windows_NT)
    VCPKG_TRIPLET ?= x64-windows
endif

all: deps
	@echo "Run submodule builds with:"
	@echo "  make -C database build-release CMAKE_TOOLCHAIN_FILE=$(VCPKG_TOOLCHAIN) VCPKG_TRIPLET=$(VCPKG_TRIPLET)"

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
	CC=clang CXX=clang++ $(VCPKG_ROOT)/vcpkg install --triplet $(VCPKG_TRIPLET)
	@echo "✓ vcpkg dependencies installed"

clean:
	rm -rf $(VCPKG_ROOT)
	rm -r ./vcpkg_installed/

help:
	@echo "Root Makefile for Falcon monorepo"
	@echo "Targets:"
	@echo "  make deps      - Install or update vcpkg"
	@echo "  make install-vcpkg-deps - Installs the dependencies"
	@echo "  make clean     - Remove vcpkg installation"
	@echo ""
	@echo "To build a submodule:"
	@echo "  make -C database build-release CMAKE_TOOLCHAIN_FILE=$(VCPKG_TOOLCHAIN) VCPKG_TRIPLET=$(VCPKG_TRIPLET)"
