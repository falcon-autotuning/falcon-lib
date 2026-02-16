# Falcon Library Root Makefile

.PHONY: all deps install-deps clean help database autotuner

VCPKG_ROOT ?= $(CURDIR)/.vcpkg
VCPKG_TOOLCHAIN ?= $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
    VCPKG_TRIPLET ?= x64-linux
endif
ifeq ($(OS),Windows_NT)
    VCPKG_TRIPLET ?= x64-windows
endif

all: deps database autotuner

help:
	@echo "Falcon Library Build System"
	@echo "============================"
	@echo ""
	@echo "Main targets:"
	@echo "  make all              - Build everything"
	@echo "  make deps             - Install vcpkg"
	@echo "  make install-deps     - Install dependencies"
	@echo "  make database         - Build database component"
	@echo "  make autotuner        - Build autotuner component"
	@echo "  make test             - Run all tests"
	@echo "  make clean            - Clean all build artifacts"
	@echo ""
	@echo "Component-specific:"
	@echo "  make -C database <target>"
	@echo "  make -C autotuner <target>"

deps:
	@if [ ! -d "$(VCPKG_ROOT)" ]; then \
		echo "Installing vcpkg to $(VCPKG_ROOT)..."; \
		git clone https://github.com/microsoft/vcpkg.git $(VCPKG_ROOT); \
		cd $(VCPKG_ROOT) && ./bootstrap-vcpkg.sh; \
	else \
		echo "vcpkg already installed at $(VCPKG_ROOT)"; \
	fi
	@echo "✓ vcpkg ready"

install-deps: deps
	@echo "Installing vcpkg dependencies..."
	cd $(VCPKG_ROOT) && ./vcpkg install \
		--triplet $(VCPKG_TRIPLET) \
		--x-install-root=$(CURDIR)/vcpkg_installed
	@echo "✓ Dependencies installed"

database:
	@$(MAKE) -C database build-release

autotuner:
	@$(MAKE) -C autotuner build-release

test:
	@$(MAKE) -C database test
	@$(MAKE) -C autotuner test

clean:
	@$(MAKE) -C database clean
	@$(MAKE) -C autotuner clean
	@echo "✓ All clean"

distclean: clean
	rm -rf $(VCPKG_ROOT)
	rm -rf vcpkg_installed
	@echo "✓ Full clean complete"
