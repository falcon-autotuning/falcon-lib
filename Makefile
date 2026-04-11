# Falcon-lib Root Makefile
# Manages build configurations for all submodules

.PHONY: all deps help clean install-vcpkg-deps build-all test-all install-deps 

VCPKG_ROOT ?= $(CURDIR)/.vcpkg
VCPKG_TOOLCHAIN ?= $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
UNAME_S := $(shell uname -s)

# GitHub release download base URL
GITHUB_RELEASE_URL = https://github.com/$(REPO)/releases/download/$(RELEASE_TAG)

PREFIX ?= /opt/falcon
LIBDIR := $(PREFIX)/lib
INCLUDEDIR := $(PREFIX)/include

ifeq ($(UNAME_S),Linux)
  VCPKG_TRIPLET ?= x64-linux-dynamic
	TMPDIR = /tmp/falcon-core-install
	SUDO ?= sudo
  ARCHIVE_CPP = falcon-core-cpp-linux-x64.tar.gz
  ARCHIVE_CPP_SHA = falcon-core-cpp-linux-x64.tar.gz.sha256
  ARCHIVE_CAPI = falcon-core-c-api-linux-x64.tar.gz
  ARCHIVE_CAPI_SHA = falcon-core-c-api-linux-x64.tar.gz.sha256
  EXTRACT_CPP = tar -xzf $(TMPDIR)/$(ARCHIVE_CPP) -C $(TMPDIR)/cpp
  EXTRACT_CAPI = tar -xzf $(TMPDIR)/$(ARCHIVE_CAPI) -C $(TMPDIR)/c_api
  EXTRACT_LSP = tar -xvf $(TMPDIR)/$(ARCHIVE_LSP) -C $(TMPDIR)/lsp
  LIBSUBDIR = lib/

endif
ifeq ($(OS),Windows_NT)
    VCPKG_TRIPLET ?= x64-windows

  TMPDIR = $(USERPROFILE)/AppData/Local/Temp/falcon-core-install
  SUDO =
	ARCHIVE_CPP = falcon-core-cpp-windows-x64.zip
  ARCHIVE_CPP_SHA := $(shell echo falcon-core-cpp-windows-x64.zip.sha256 | tr -d '\r')
  ARCHIVE_CAPI = falcon-core-c-api-windows-x64.zip
  ARCHIVE_CAPI_SHA := $(shell echo falcon-core-c-api-windows-x64.zip.sha256 | tr -d '\r')
  EXTRACT_CPP = unzip -o $(TMPDIR)/$(ARCHIVE_CPP) -d $(TMPDIR)/cpp
  EXTRACT_CAPI = unzip -o $(TMPDIR)/$(ARCHIVE_CAPI) -d $(TMPDIR)/c_api
  LIBSUBDIR = bin/
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

install-vcpkg-deps: 
	@echo "Installing vcpkg dependencies..."
	@if [ ! -z "$(NUGET_API_KEY)" ] && [ ! -z "$(NUGET_FEED_URL)" ]; then \
		echo "Generating temporary nuget.config for binary caching..."; \
		printf '<?xml version="1.0" encoding="utf-8"?>\n<configuration>\n  <config>\n    <add key="defaultPushSource" value="cpp-cache" />\n  </config>\n  <packageSources>\n    <add key="cpp-cache" value="%s" />\n  </packageSources>\n  <packageSourceCredentials>\n    <cpp-cache>\n      <add key="Username" value="az" />\n      <add key="ClearTextPassword" value="%s" />\n    </cpp-cache>\n  </packageSourceCredentials>\n  <apikeys>\n    <add key="cpp-cache" value="AzureDevOps" />\n  </apikeys>\n</configuration>\n' "$(NUGET_FEED_URL)" "$(NUGET_API_KEY)" > $(VCPKG_ROOT)/nuget.config; \
		export VCPKG_BINARY_SOURCES="clear;nugetconfig,$(VCPKG_ROOT)/nuget.config,readwrite"; \
	elif [ ! -z "$(VCPKG_BINARY_SOURCES)" ]; then \
		echo "Using binary sources: $(VCPKG_BINARY_SOURCES)"; \
	fi && \
	CC=clang CXX=clang++ MAKELEVEL=0 $(VCPKG_ROOT)/vcpkg install --triplet $(VCPKG_TRIPLET) --overlay-ports=./ports
	@echo "Patching cereal install..."
	$(SUDO) mkdir -p $(INCLUDEDIR)/cereal/types
	$(SUDO) cp $(CURDIR)/vcpkg_installed/$(VCPKG_TRIPLET)/include/cereal/types/xtensor.hpp $(INCLUDEDIR)/cereal/types/xtensor.hpp
	@echo "✓ vcpkg dependencies installed"

install:
	@echo "Installing all components..."
	$(MAKE) -C database install SUDO="$(SUDO)"
	$(MAKE) -C comms install SUDO="$(SUDO)"
	# $(MAKE) -C typing install SUDO="$(SUDO)"
	#$(MAKE) -C routine install SUDO="$(SUDO)"
	$(MAKE) -C qarrayDevice install-python-deps SUDO="$(SUDO)"
	$(MAKE) -C qarrayDevice install SUDO="$(SUDO)"
	$(MAKE) -C dsl install SUDO="$(SUDO)"
	@echo "✓ All components installed"

clean:
	@echo "Cleaning all components..."
	$(MAKE) -C database clean
	$(MAKE) -C comms clean
	$(MAKE) -C typing clean
	$(MAKE) -C routine clean
	$(MAKE) -C qarrayDevice clean
	$(MAKE) -C dsl clean
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
	@echo "  make install-core       - Install falcon_core"
	@echo ""
	@echo "Build targets:"
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

# ==========================================
# Docker & Database Configuration Targets
# ==========================================

DOCKER_IMAGE ?= falcon-cli:latest
DB_CONTAINER_NAME ?= falcon-postgres
DB_PORT ?= 5432
CONFIG_VOLUME ?= falcon-config

.PHONY: docker-build docker-db-start docker-db-stop docker-install-wrappers docker-uninstall-wrappers docker-teardown

docker-build:
	@echo "Building FAlCon Docker Image..."
	docker build -t $(DOCKER_IMAGE) -f packaging/docker/Dockerfile .
	@echo "✓ Docker image $(DOCKER_IMAGE) built successfully."

docker-db-start:
	@echo "Creating secure volume..."
	@docker volume create $(CONFIG_VOLUME) > /dev/null
	@echo "Launching secure configuration prompt..."
	@# 1. Spin up an interactive bash container. 
	@# It captures input securely (hiding the password) and writes to the volume.
	@docker run -it --rm -v $(CONFIG_VOLUME):/config bash -c "\
		echo '=== FAlCon Database Setup ===' && \
		read -p 'Database Username [falcon_user]: ' usr && usr=\$${usr:-falcon_user} && \
		read -s -p 'Database Password: ' pass && echo && \
		read -p 'Database Name [falcon_db]: ' dbname && dbname=\$${dbname:-falcon_db} && \
		echo \"\$$usr\" > /config/db_user.txt && \
		echo \"\$$pass\" > /config/db_pass.txt && \
		echo \"\$$dbname\" > /config/db_name.txt && \
		echo \"export FALCON_DB_URL=postgresql://\$$usr:\$$pass@host.docker.internal:$(DB_PORT)/\$$dbname\" > /config/db.env && \
		echo '✓ Credentials securely stored in Docker volume.'"
	@echo "Starting PostgreSQL container..."
	@# 2. Start Postgres using the _FILE variables, pointing to the volume.
	@docker run -d --name $(DB_CONTAINER_NAME) \
		-v $(CONFIG_VOLUME):/config:ro \
		-e POSTGRES_USER_FILE=/config/db_user.txt \
		-e POSTGRES_PASSWORD_FILE=/config/db_pass.txt \
		-e POSTGRES_DB_FILE=/config/db_name.txt \
		-p $(DB_PORT):5432 \
		postgres:15
	@echo "✓ Database started securely."

docker-db-stop:
	@echo "Stopping database and destroying secure configuration..."
	-docker stop $(DB_CONTAINER_NAME)
	-docker rm $(DB_CONTAINER_NAME)
	-docker volume rm $(CONFIG_VOLUME)
	@echo "✓ Database and credentials destroyed."

docker-install-wrappers:
	@if [ "$(UNAME_S)" = "Linux" ] || [ "$(UNAME_S)" = "Darwin" ]; then \
		echo "Installing wrapper scripts to /usr/local/bin..."; \
		$(SUDO) cp packaging/wrappers/linux_mac/*.sh /usr/local/bin/; \
		$(SUDO) rename 's/\.sh$$//' /usr/local/bin/falcon-*.sh 2>/dev/null || \
			for f in /usr/local/bin/falcon-*.sh; do $(SUDO) mv "$$f" "$${f%.sh}"; done; \
		$(SUDO) chmod +x /usr/local/bin/falcon-*; \
		echo "✓ Wrappers installed. You can now run 'falcon-run', etc."; \
	else \
		echo "For Windows, please manually add 'packaging/wrappers/windows' to your PATH."; \
	fi

docker-uninstall-wrappers:
	@if [ "$(UNAME_S)" = "Linux" ] || [ "$(UNAME_S)" = "Darwin" ]; then \
		echo "Removing wrapper scripts from /usr/local/bin..."; \
		$(SUDO) rm -f /usr/local/bin/falcon-run /usr/local/bin/falcon-test /usr/local/bin/falcon-pm /usr/local/bin/falcon-db-cli; \
		echo "✓ Wrappers uninstalled."; \
	else \
		echo "For Windows, please manually remove the folder from your PATH."; \
	fi

docker-teardown: docker-db-stop docker-uninstall-wrappers
	@echo "Removing FAlCon Docker image..."
	-docker rmi $(DOCKER_IMAGE)
	@echo "✓ Teardown complete."
