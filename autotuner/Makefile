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

INSTALL_PREFIX ?= /opt/falcon
INSTALL_LIBDIR := $(INSTALL_PREFIX)/lib
INSTALL_INCLUDEDIR := $(INSTALL_PREFIX)/include

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
		-Dfalcon_core_DIR=/opt/falcon/include/cmake \
		-Dfalcon-database_DIR=/opt/falcon/lib/cmake/falcon-database \
		-Dfalcon-comms_DIR=/opt/falcon/lib/cmake/falcon-comms \
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
    -Dfalcon_core_DIR=/opt/falcon/include/cmake \
    -Dfalcon-database_DIR=/opt/falcon/lib/cmake/falcon-database \
    -Dfalcon-comms_DIR=/opt/falcon/lib/cmake/falcon-comms \
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


run-voltage-sweep: examples
	@echo "Running voltage sweep example..."
	cd $(BUILD_DIR_RELEASE)/examples/voltage_sweep && \
		../../falcon-runtime ./libvoltage_sweep_autotuner.so \
		--entry VoltageSweep \
		--snapshot ../../../examples/voltage_sweep/snapshot.json \
		--analyze

install: build-release
	@echo "Installing falcon-database to $(INSTALL_PREFIX)..."
	$(SUDO) cmake --install $(BUILD_DIR_RELEASE) --prefix $(INSTALL_PREFIX)
	@echo "Copying vcpkg headers and libraries..."
	$(SUDO) cp -r $(VCPKG_INSTALLED_DIR)/$(VCPKG_TRIPLET)/include/* $(INSTALL_INCLUDEDIR)/
	$(SUDO) cp -r $(VCPKG_INSTALLED_DIR)/$(VCPKG_TRIPLET)/lib/* $(INSTALL_LIBDIR)/
	$(SUDO) cp -r $(VCPKG_INSTALLED_DIR)/$(VCPKG_TRIPLET)/bin/* $(INSTALL_LIBDIR)/ || true
	@echo "✓ Installation complete"

clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR_DEBUG) $(BUILD_DIR_RELEASE) build/ compile_commands.json
	@$(MAKE) docker-clean
	@echo "✓ Clean complete"

.PHONY: clangd-helpers
clangd-helpers:
	@if [ -f $(BUILD_DIR_DEBUG)/compile_commands.json ]; then \
		ln -sf $(BUILD_DIR_DEBUG)/compile_commands.json compile_commands.json; \
		echo "✓ clangd compile_commands.json symlinked to autotuners/ root"; \
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
	@cd $(BUILD_DIR_DEBUG) && \
		TEST_DATABASE_URL="$(TEST_DATABASE_URL)" \
		TEST_NATS_URL="$(TEST_NATS_URL)" \
		ctest --verbose -C Debug
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
