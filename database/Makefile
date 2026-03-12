# Falcon Database Makefile
# Manages build configurations and testing using vcpkg

.PHONY: all configure build-debug build-release test clean install help check-vcpkg

# Detect OS
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PLATFORM := linux
    CMAKE_GENERATOR := Ninja
    VCPKG_TRIPLET ?= x64-linux-dynamic
    NPROC := $(shell nproc 2>/dev/null || echo 4)
    HAS_DOCKER := $(shell command -v docker-compose >/dev/null 2>&1 && echo yes || echo no)
    SUDO := sudo
endif
ifeq ($(OS),Windows_NT)
    PLATFORM := windows
    CMAKE_GENERATOR := "Visual Studio 17 2022"
    VCPKG_TRIPLET ?= x64-windows
    NPROC := 4
    HAS_DOCKER := $(shell where docker-compose >nul 2>&1 && echo yes || echo no)
    SUDO :=
endif

# Paths (relative to database/ directory)
VCPKG_ROOT ?= $(CURDIR)/../.vcpkg
VCPKG_TOOLCHAIN ?= $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
VCPKG_INSTALLED_DIR ?= $(CURDIR)/../vcpkg_installed

BUILD_DIR_DEBUG := build/debug
BUILD_DIR_RELEASE := build/release

INSTALL_PREFIX ?= /opt/falcon
INSTALL_LIBDIR := $(INSTALL_PREFIX)/lib
INSTALL_INCLUDEDIR := $(INSTALL_PREFIX)/include

# Default target
all: build-release

help:
	@echo "Falcon Database Build System"
	@echo "============================="
	@echo ""
	@echo "Build targets:"
	@echo "  make build-debug    - Build debug version"
	@echo "  make build-release  - Build release version"
	@echo "  make configure      - Configure both debug and release builds"
	@echo "  make clean          - Clean build artifacts"
	@echo "  make install        - Install the library"
	@echo ""
	@echo "Test targets (require PostgreSQL):"
	@echo "  make test           - Run tests (uses Docker by default)"
	@echo "  make test-debug     - Run debug tests (uses Docker by default)"
	@echo "  make test-local     - Run tests with local PostgreSQL"
	@echo "  make test-verbose   - Run tests with verbose output"
	@echo ""
	@echo "Docker targets:"
	@echo "  make docker-up      - Start PostgreSQL test database"
	@echo "  make docker-down    - Stop PostgreSQL test database"
	@echo "  make docker-clean   - Clean PostgreSQL test data"
	@echo "  make docker-logs    - Show PostgreSQL logs"
	@echo ""
	@echo "Environment variables:"
	@echo "  VCPKG_ROOT          - Path to vcpkg root (default: ../.vcpkg)"
	@echo "  VCPKG_TRIPLET       - vcpkg triplet (default: x64-linux-dynamic)"
	@echo "  TEST_DATABASE_URL   - PostgreSQL connection for tests"
	@echo "  USE_LOCAL_DB=1      - Force using local PostgreSQL instead of Docker"
	@echo ""
	@echo "Current configuration:"
	@echo "  Platform: $(PLATFORM)"
	@echo "  Generator: $(CMAKE_GENERATOR)"
	@echo "  Triplet: $(VCPKG_TRIPLET)"
	@echo "  Docker available: $(HAS_DOCKER)"

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

check-docker:
	@if [ "$(HAS_DOCKER)" = "no" ]; then \
		echo "Error: docker-compose not found"; \
		echo "Install Docker Desktop or docker-compose to run tests"; \
		echo "Or set USE_LOCAL_DB=1 to use local PostgreSQL"; \
		exit 1; \
	fi

configure-debug: check-vcpkg
	@echo "Configuring debug build..."
	@mkdir -p $(BUILD_DIR_DEBUG)
	cd $(BUILD_DIR_DEBUG) && cmake ../.. \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_TOOLCHAIN_FILE=$(VCPKG_TOOLCHAIN) \
		-DVCPKG_INSTALLED_DIR=$(VCPKG_INSTALLED_DIR) \
		-DVCPKG_TARGET_TRIPLET=$(VCPKG_TRIPLET) \
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
		-DBUILD_TESTS=ON \
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

# Docker test targets (default)
.PHONY: docker-up docker-down docker-clean docker-logs test-with-docker

docker-up: check-docker
	@echo "Starting PostgreSQL test database..."
	@docker-compose -f docker-compose.test.yml up -d
	@echo "Waiting for PostgreSQL to be ready..."
	@for i in 1 2 3 4 5; do \
		if docker-compose -f docker-compose.test.yml exec -T postgres pg_isready -U falcon_test >/dev/null 2>&1; then \
			echo "✓ PostgreSQL is ready"; \
			exit 0; \
		fi; \
		echo "  Waiting... ($$i/5)"; \
		sleep 2; \
	done; \
	echo "✗ PostgreSQL failed to start"; \
	exit 1

docker-down:
	@echo "Stopping PostgreSQL test database..."
	@docker-compose -f docker-compose.test.yml down
	@echo "✓ PostgreSQL stopped"

docker-clean:
	@echo "Cleaning PostgreSQL test data..."
	@docker-compose -f docker-compose.test.yml down -v
	@echo "✓ PostgreSQL data cleaned"

docker-logs:
	@docker-compose -f docker-compose.test.yml logs -f postgres

# Test with Docker (default behavior)
test-with-docker: docker-up build-release
	@echo "Running tests with Docker PostgreSQL..."
	@TEST_DATABASE_URL="postgresql://falcon_test:falcon_test_password@localhost:5432/falcon_test" \
		$(MAKE) test-local || ($(MAKE) docker-down && exit 1)
	@$(MAKE) docker-down
	@echo "✓ Tests complete, database stopped"

test-debug-docker: docker-up build-debug
	@echo "Running debug tests with Docker PostgreSQL..."
	@TEST_DATABASE_URL="postgresql://falcon_test:falcon_test_password@localhost:5432/falcon_test" \
		$(MAKE) test-local-debug || ($(MAKE) docker-down && exit 1)
	@$(MAKE) docker-down
	@echo "✓ Tests complete, database stopped"

# Keep database running for interactive testing
test-interactive: docker-up build-debug
	@echo "Running tests (debug) with Docker PostgreSQL..."
	@echo "Database will remain running for inspection"
	@TEST_DATABASE_URL="postgresql://falcon_test:falcon_test_password@localhost:5432/falcon_test" \
		cd $(BUILD_DIR_DEBUG) && ctest --output-on-failure -C Debug
	@echo ""
	@echo "Database is still running. To stop it, run: make docker-down"
	@echo "To see logs: make docker-logs"

# Local tests (without Docker) - requires local PostgreSQL
test-local: build-release
	@echo "Running tests with local PostgreSQL..."
	@if [ -z "$(TEST_DATABASE_URL)" ]; then \
		echo "Warning: TEST_DATABASE_URL not set"; \
		echo "Using default: postgresql://localhost/falcon_test"; \
		echo "Set TEST_DATABASE_URL to use a different connection"; \
	fi
	cd $(BUILD_DIR_RELEASE) && ctest --output-on-failure -C Release
	@echo "✓ All tests passed"

test-local-debug: build-debug
	@echo "Running debug tests with local PostgreSQL..."
	@if [ -z "$(TEST_DATABASE_URL)" ]; then \
		echo "Warning: TEST_DATABASE_URL not set"; \
	fi
	cd $(BUILD_DIR_DEBUG) && ctest --output-on-failure -C Debug
	@echo "✓ All tests passed"

test-verbose: docker-up build-release
	@echo "Running tests with verbose output..."
	@TEST_DATABASE_URL="postgresql://falcon_test:falcon_test_password@localhost:5432/falcon_test" \
		cd $(BUILD_DIR_RELEASE) && ctest --verbose -C Release || true
	@$(MAKE) docker-down

# Default test targets - use Docker if available
ifeq ($(USE_LOCAL_DB),1)
test: test-local
test-debug: test-local-debug
else ifeq ($(HAS_DOCKER),yes)
test: test-with-docker
test-debug: test-debug-docker
else
test: test-local
	@echo ""
	@echo "Note: Install Docker to automatically manage test database"
test-debug: test-local-debug
	@echo ""
	@echo "Note: Install Docker to automatically manage test database"
endif

install: build-release
	@echo "Installing falcon-database to $(INSTALL_PREFIX)..."
	$(SUDO) cmake --install $(BUILD_DIR_RELEASE) --prefix $(INSTALL_PREFIX)
	$(SUDO) cp -v $(BUILD_DIR_RELEASE)/falcon-db-cli $(INSTALL_LIBDIR)/falcon-db-cli
	@echo "Copying vcpkg headers and libraries..."
	$(SUDO) cp -r $(VCPKG_INSTALLED_DIR)/$(VCPKG_TRIPLET)/include/* $(INSTALL_INCLUDEDIR)/
	$(SUDO) cp -r $(VCPKG_INSTALLED_DIR)/$(VCPKG_TRIPLET)/lib/* $(INSTALL_LIBDIR)/
	$(SUDO) cp -r $(VCPKG_INSTALLED_DIR)/$(VCPKG_TRIPLET)/bin/* $(INSTALL_LIBDIR)/ || true
	@echo "✓ Installation complete"

uninstall:
	@echo "Uninstalling falcon-database from $(INSTALL_PREFIX)..."
	$(SUDO) rm -r $(INSTALL_LIBDIR)/libfalcon-database* $(INSTALL_INCLUDEDIR)/falcon-database* $(INSTALL_PREFIX)/lib/cmake/falcon-database $(INSTALL_LIBDIR)/falcon-db-cli || true
	@echo "✓ Uninstall complete"

clean: uninstall
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
		echo "✓ clangd compile_commands.json symlinked to database/ root"; \
	else \
		echo "No compile_commands.json found in debug build directory."; \
	fi
