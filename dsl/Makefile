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
    SUDO := sudo
endif
ifeq ($(OS),Windows_NT)
    PLATFORM := windows
    CMAKE_GENERATOR := "Visual Studio 17 2022"
    VCPKG_TRIPLET ?= x64-windows
    NPROC := 4
SUDO := 
endif

# Paths
VCPKG_ROOT ?= $(CURDIR)/../.vcpkg
VCPKG_TOOLCHAIN ?= $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
VCPKG_INSTALLED_DIR ?= $(CURDIR)/../vcpkg_installed

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
		-DCOMPILE_OPTIONS="-g " \
		-Dfalcon_core_DIR=/opt/falcon/lib/cmake/falcon_core \
		-Dfalcon-database_DIR=/opt/falcon/lib/cmake/falcon-database \
		-Dfalcon-comms_DIR=/opt/falcon/lib/cmake/falcon-comms \
		-Dlsp_DIR=/opt/falcon/lib/cmake/lsp \
		-Dfalcon-typing_DIR=/opt/falcon/lib/cmake/falcon-typing \
		-DBUILD_TESTS=ON \
		-DBUILD_TOOLS=ON \
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
		-Dfalcon_core_DIR=/opt/falcon/lib/cmake/falcon_core \
		-Dfalcon-database_DIR=/opt/falcon/lib/cmake/falcon-database \
		-Dfalcon-comms_DIR=/opt/falcon/lib/cmake/falcon-comms \
		-Dlsp_DIR=/opt/falcon/lib/cmake/lsp \
		-Dfalcon-typing_DIR=/opt/falcon/lib/cmake/falcon-typing \
		-DBUILD_TESTS=ON \
		-DBUILD_TOOLS=ON \
		-DUSE_CCACHE=ON \
		-DENABLE_PCH=ON \
		-DCMAKE_C_COMPILER=clang \
		-DCMAKE_CXX_COMPILER=clang++ \
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
	@echo "✓ falcon-dsl installed"
	@echo "  Library : $(INSTALL_LIBDIR)/libfalcon-dsl.so"
	@echo "  Headers : $(INSTALL_INCLUDEDIR)/falcon-dsl/"
	@echo "  CMake   : $(INSTALL_CMAKEDIR)/"
	@echo "  CLI     : $(INSTALL_LIBDIR)/falcon-run"

uninstall: uninstall-lsp
	@echo "Uninstalling falcon-dsl from $(INSTALL_PREFIX)..."
	$(SUDO) rm -f  $(INSTALL_LIBDIR)/libfalcon-dsl.so
	$(SUDO) rm -f  $(INSTALL_LIBDIR)/libfalcon-dsl.so.1
	$(SUDO) rm -f  $(INSTALL_LIBDIR)/libfalcon-dsl.so.1.0.0
	$(SUDO) rm -r $(INSTALL_INCLUDEDIR)/falcon-dsl
	$(SUDO) rm -r $(INSTALL_CMAKEDIR)
	$(SUDO) rm -f $(INSTALL_PREFIX)/bin/falcon-run
	@echo "✓ Uninstall complete"

uninstall-lsp:
	@echo "Uninstalling lsp library from $(INSTALL_PREFIX)..."
	$(SUDO) rm -f $(INSTALL_LIBDIR)/libfalcon-lsp.a
	$(SUDO) rm -f $(INSTALL_LIBDIR)/falcon-lsp
	$(SUDO) rm -r $(INSTALL_INCLUDEDIR)/falcon-lsp
	@echo "✓ lsp library uninstalled"

install-pm: build-release
	@echo "Installing falcon-pm (package manager) to $(INSTALL_PREFIX)..."
	$(SUDO) mkdir -p $(INSTALL_LIBDIR)
	$(SUDO) mkdir -p $(INSTALL_INCLUDEDIR)/falcon-pm
	$(SUDO) cp -v $(BUILD_DIR_RELEASE)/package-manager/libfalcon-pm.a $(INSTALL_LIBDIR)/ 2>/dev/null || true
	$(SUDO) find $(BUILD_DIR_RELEASE)/package-manager -name "falcon-pm" -type f \
		-exec cp -v {} $(INSTALL_PREFIX)/bin/falcon-pm \; 2>/dev/null || true
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
	$(SUDO) rm -r $(INSTALL_INCLUDEDIR)/falcon-dsl
	$(SUDO) rm -r $(INSTALL_CMAKEDIR)
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
	rm -rf $(BUILD_DIR_DEBUG) $(BUILD_DIR_RELEASE) build/ compile_commands.json
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

# Docker test targets
.PHONY: docker-up docker-down docker-clean docker-logs test-with-docker

check-docker:
	@if ! command -v docker-compose >/dev/null 2>&1; then \
		echo "Error: docker-compose not found"; \
		echo "Install Docker Desktop or docker-compose to run tests"; \
		exit 1; \
	fi

docker-up: check-docker
	@echo "Starting test services (PostgreSQL + NATS)..."
	@docker-compose -f docker-compose.test.yml up -d
	@echo "Waiting for services to be ready..."
	@for i in 1 2 3 4 5; do \
		if docker-compose -f docker-compose.test.yml exec -T postgres pg_isready -U falcon_test >/dev/null 2>&1 && \
		   docker-compose -f docker-compose.test.yml exec -T nats wget -q -O- http://localhost:8222/healthz >/dev/null 2>&1; then \
			echo "✓ Services are ready"; \
			exit 0; \
		fi; \
		echo "  Waiting... ($$i/5)"; \
		sleep 3; \
	done; \
	echo "✗ Services failed to start"; \
	exit 1

docker-down:
	@echo "Stopping test services..."
	@docker-compose -f docker-compose.test.yml down
	@echo "✓ Services stopped"

docker-clean:
	@echo "Cleaning test data..."
	@docker-compose -f docker-compose.test.yml down -v
	@echo "✓ Test data cleaned"

docker-logs:
	@docker-compose -f docker-compose.test.yml logs -f

# Test with Docker (recommended)
test-with-docker: docker-up build-release
	@echo "Running tests with Docker services..."
	@$(MAKE) test-local \
		TEST_DATABASE_URL="postgresql://falcon_test:falcon_test_password@127.0.0.1:5433/falcon_test" \
		TEST_NATS_URL="nats://localhost:4222"
	@$(MAKE) docker-down
	@echo "✓ Tests complete, services stopped"

test-debug-docker: docker-up build-debug
	@echo "Running debug tests with Docker services..."
	@$(MAKE) test-local-debug \
		TEST_DATABASE_URL="postgresql://falcon_test:falcon_test_password@127.0.0.1:5433/falcon_test" \
		TEST_NATS_URL="nats://localhost:4222"
	@$(MAKE) docker-down
	@echo "✓ Tests complete, services stopped"

# Local tests (assumes services already running)
test-local: build-release
	@echo "Running tests with local services..."
	@if [ -z "$(TEST_DATABASE_URL)" ]; then \
		echo "⚠ Warning: TEST_DATABASE_URL not set"; \
		echo "  Set it with: export TEST_DATABASE_URL=postgresql://..."; \
		exit 1; \
	fi
	@if [ -z "$(TEST_NATS_URL)" ]; then \
		echo "⚠ Warning: TEST_NATS_URL not set"; \
		echo "  Set it with: export TEST_NATS_URL=nats://localhost:4222"; \
		exit 1; \
	fi
	@echo "Using TEST_DATABASE_URL=$(TEST_DATABASE_URL)"
	@echo "Using TEST_NATS_URL=$(TEST_NATS_URL)"
	@cd $(BUILD_DIR_RELEASE) && \
		TEST_DATABASE_URL="$(TEST_DATABASE_URL)" \
		TEST_NATS_URL="$(TEST_NATS_URL)" \
		ctest --verbose -C Release
	@echo "✓ All tests passed"

test-local-debug: build-debug
	@echo "Running debug tests with local services..."
	@if [ -z "$(TEST_DATABASE_URL)" ]; then \
		echo "⚠ Warning: TEST_DATABASE_URL not set"; \
		exit 1; \
	fi
	@if [ -z "$(TEST_NATS_URL)" ]; then \
		echo "⚠ Warning: TEST_NATS_URL not set"; \
		exit 1; \
	fi
	@cd $(BUILD_DIR_DEBUG)/package-manager && \
		TEST_DATABASE_URL="$(TEST_DATABASE_URL)" \
		TEST_NATS_URL="$(TEST_NATS_URL)" \
		./falcon-pm-tests $(if $(GTEST_FILTER),--gtest_filter="$(GTEST_FILTER)",)
	@echo "Running dsl tests..."
	@cd $(BUILD_DIR_DEBUG)/tests && \
		LD_LIBRARY_PATH=$(CURDIR)/$(BUILD_DIR_DEBUG):$(VCPKG_INSTALLED_DIR)/$(VCPKG_TRIPLET)/lib:$(INSTALL_PREFIX)/lib:$$LD_LIBRARY_PATH \
		TEST_DATABASE_URL="$(TEST_DATABASE_URL)" \
		TEST_NATS_URL="$(TEST_NATS_URL)" \
		./falcon_dsl_integration_tests $(if $(GTEST_FILTER),--gtest_filter="$(GTEST_FILTER)",)
	@echo "Running lsp tests..."
	@cd $(BUILD_DIR_DEBUG)/lsp/tests && \
		./falcon_lsp_unit_tests $(if $(GTEST_FILTER),--gtest_filter="$(GTEST_FILTER)",) && \
		./falcon_lsp_integration_tests $(if $(GTEST_FILTER),--gtest_filter="$(GTEST_FILTER)",)
	@echo "✓ All tests passed"

# Keep services running for interactive testing
test-interactive: docker-up build-debug
	@echo "Services are running. Run tests manually with:"
	@echo "  export TEST_DATABASE_URL=postgresql://falcon_test:falcon_test_password@127.0.0.1:5433/falcon_test"
	@echo "  export TEST_NATS_URL=nats://localhost:4222"
	@echo "  cd $(BUILD_DIR_DEBUG) && ctest"
	@echo ""
	@echo "Or run individual test executables:"
	@echo "  $(BUILD_DIR_DEBUG)/tests/falcon_routine_unit_tests"
	@echo "  $(BUILD_DIR_DEBUG)/tests/falcon_routine_integration_tests"
	@echo ""
	@echo "When done, stop services with: make docker-down"

# Override default test targets to use Docker
test: test-with-docker
test-debug: test-debug-docker


