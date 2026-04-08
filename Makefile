# Falcon-lib Root Makefile
# Manages build configurations for all submodules

.PHONY: all deps help clean install-vcpkg-deps build-all test-all install-lsp-framework install-deps install-core

VCPKG_ROOT ?= $(CURDIR)/.vcpkg
VCPKG_TOOLCHAIN ?= $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
UNAME_S := $(shell uname -s)

# Repo and release configuration
REPO = falcon-autotuning/falcon-core
RELEASE_TAG = v1.1.0
LIBS_RELEASE_TAG = v0.0.2
LIBS_REPO = falcon-autotuning/falcon-core-libs

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


install-core:
	@echo "Fetching latest release assets from GitHub..."
	mkdir -p $(TMPDIR)
	$(SUDO) mkdir -p $(LIBDIR)
	$(SUDO) mkdir -p $(INCLUDEDIR)
	@echo "Downloading $(ARCHIVE_CPP)..."
	curl -L -f $(if $(GITHUB_TOKEN),-H "Authorization: token $(GITHUB_TOKEN)",) -o $(TMPDIR)/$(ARCHIVE_CPP) \
		$(GITHUB_RELEASE_URL)/$(ARCHIVE_CPP)
	@echo "Downloading $(ARCHIVE_CPP_SHA)..."
	curl -L -f $(if $(GITHUB_TOKEN),-H "Authorization: token $(GITHUB_TOKEN)",) -o $(TMPDIR)/$(ARCHIVE_CPP_SHA) \
		$(GITHUB_RELEASE_URL)/$(ARCHIVE_CPP_SHA)
	@echo "Downloading $(ARCHIVE_CAPI)..."
	curl -L -f $(if $(GITHUB_TOKEN),-H "Authorization: token $(GITHUB_TOKEN)",) -o $(TMPDIR)/$(ARCHIVE_CAPI) \
		$(GITHUB_RELEASE_URL)/$(ARCHIVE_CAPI)
	@echo "Downloading $(ARCHIVE_CAPI_SHA)..."
	curl -L -f $(if $(GITHUB_TOKEN),-H "Authorization: token $(GITHUB_TOKEN)",) -o $(TMPDIR)/$(ARCHIVE_CAPI_SHA) \
		$(GITHUB_RELEASE_URL)/$(ARCHIVE_CAPI_SHA)
ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
	dos2unix "$(TMPDIR)/falcon-core-cpp-windows-x64.zip.sha256"
	dos2unix "$(TMPDIR)/falcon-core-c-api-windows-x64.zip.sha256"
endif
	@echo "Verifying checksums..."
	cd "$(TMPDIR)" && sha256sum -c "$(shell echo $(ARCHIVE_CPP_SHA) | tr -d '\r')"
	cd "$(TMPDIR)" && sha256sum -c "$(shell echo $(ARCHIVE_CAPI_SHA) | tr -d '\r')"
	@echo "Extracting Archives..."
	mkdir -p $(TMPDIR)/cpp
	mkdir -p $(TMPDIR)/c_api
	$(EXTRACT_CPP)
	$(EXTRACT_CAPI)
	@echo "Installing Shared Libraries..."
	$(SUDO) install -Dm755 $(TMPDIR)/cpp/$(LIBSUBDIR)* $(LIBDIR)/
	$(SUDO) install -Dm755 $(TMPDIR)/c_api/$(LIBSUBDIR)* $(LIBDIR)/
	@echo "Extracting and Installing C++ Headers..."
	$(SUDO) mkdir -p $(INCLUDEDIR)/falcon-core-cpp/falcon_core/
	$(SUDO) cp -r $(TMPDIR)/cpp/include/falcon_core/* $(INCLUDEDIR)/falcon-core-cpp/falcon_core/
	@echo "Extracting and Installing C API Headers..."
	$(SUDO) mkdir -p $(INCLUDEDIR)/falcon-core-c-api/falcon_core/
	$(SUDO) cp -r $(TMPDIR)/c_api/include/falcon_core/* $(INCLUDEDIR)/falcon-core-c-api/falcon_core/
	@echo "Installing other Headers..."
	$(SUDO) find $(TMPDIR)/cpp/include -mindepth 1 -maxdepth 1 ! -name 'falcon_core' -exec cp -r {} $(INCLUDEDIR)/ \;
	$(SUDO) find $(TMPDIR)/c_api/include -mindepth 1 -maxdepth 1 ! -name 'falcon_core' -exec cp -r {} $(INCLUDEDIR)/ \;
ifeq ($(UNAME_S),Linux)
	@echo "Updating linker cache..."
	$(SUDO) ldconfig
endif
	@echo "falcon-core libraries and headers installed successfully."
	$(SUDO) mkdir -p $(INCLUDEDIR)/falcon_core 
	$(SUDO) cp -r $(INCLUDEDIR)/falcon-core-cpp/falcon_core/* $(INCLUDEDIR)/falcon_core
	$(SUDO) cp -r $(INCLUDEDIR)/falcon-core-c-api/falcon_core/* $(INCLUDEDIR)/falcon_core
	$(SUDO) rm -rf $(INCLUDEDIR)/falcon-core-cpp
	$(SUDO) rm -rf $(INCLUDEDIR)/falcon-core-c-api
	@echo "Installing cmake files for falcon_core that are not included with the build"
	$(SUDO) mkdir -p $(LIBDIR)/cmake
	$(SUDO) mkdir -p $(LIBDIR)/cmake/falcon_core
	$(SUDO) cp ./falcon_core-config-version.cmake $(LIBDIR)/cmake/falcon_core
	$(SUDO) cp ./falcon_core-config.cmake $(LIBDIR)/cmake/falcon_core
	@echo ""


install-vcpkg-deps: 
	@echo "Installing vcpkg dependencies..."
	@if [ ! -z "$(NUGET_API_KEY)" ] && [ ! -z "$(NUGET_FEED_URL)" ]; then \
		echo "Generating temporary nuget.config for binary caching..."; \
		printf '<?xml version="1.0" encoding="utf-8"?>\n<configuration>\n  <packageSources>\n    <add key="AzureDevOps" value="%s" />\n  </packageSources>\n  <packageSourceCredentials>\n    <AzureDevOps>\n      <add key="Username" value="az" />\n      <add key="ClearTextPassword" value="%s" />\n    </AzureDevOps>\n  </packageSourceCredentials>\n</configuration>\n' "$(NUGET_FEED_URL)" "$(NUGET_API_KEY)" > $(VCPKG_ROOT)/nuget.config; \
		export VCPKG_BINARY_SOURCES="clear;nugetconfig,$(VCPKG_ROOT)/nuget.config,readwrite"; \
	elif [ ! -z "$(VCPKG_BINARY_SOURCES)" ]; then \
		echo "Using binary sources: $(VCPKG_BINARY_SOURCES)"; \
	fi && \
	CC=clang CXX=clang++ MAKELEVEL=0 $(VCPKG_ROOT)/vcpkg install --triplet $(VCPKG_TRIPLET)
	@echo "Patching cereal install..."
	mkdir -p $(CURDIR)/vcpkg_installed/$(VCPKG_TRIPLET)/include/cereal/types
	curl -sSL $(if $(GITHUB_TOKEN),-H "Authorization: token $(GITHUB_TOKEN)",) https://raw.githubusercontent.com/falcon-autotuning/falcon-core/main/cpp/include/cereal/types/xtensor.hpp -o $(CURDIR)/vcpkg_installed/$(VCPKG_TRIPLET)/include/cereal/types/xtensor.hpp
	$(SUDO) mkdir -p $(INCLUDEDIR)/cereal/types
	$(SUDO) cp $(CURDIR)/vcpkg_installed/$(VCPKG_TRIPLET)/include/cereal/types/xtensor.hpp $(INCLUDEDIR)/cereal/types/xtensor.hpp
	@echo "✓ vcpkg dependencies installed"

install-lsp-framework: install-vcpkg-deps
	@echo "Installing lsp-framework 1.3.0 from source..."
	mkdir -p $(TMPDIR)
	curl -L -f $(if $(GITHUB_TOKEN),-H "Authorization: token $(GITHUB_TOKEN)",) -o $(TMPDIR)/lsp-framework-1.3.0.zip https://github.com/leon-bckl/lsp-framework/archive/refs/tags/1.3.0.zip
	unzip -o $(TMPDIR)/lsp-framework-1.3.0.zip -d $(TMPDIR)
	@echo "Building lsp-framework (Release) with clang..."
	cd $(TMPDIR)/lsp-framework-1.3.0 && mkdir -p build && cd build && CC=clang CXX=clang++ cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$(PREFIX)
	cd $(TMPDIR)/lsp-framework-1.3.0/build && make
	@echo "Installing lsp-framework (Release)..."
	cd $(TMPDIR)/lsp-framework-1.3.0/build && cmake --install . --prefix $(TMPDIR)/lsp-framework-1.3.0/build
	$(SUDO) install -Dm755 $(TMPDIR)/lsp-framework-1.3.0/build/liblsp.a $(LIBDIR)/
	@echo "Building lsp-framework (Debug) with clang..."
	cd $(TMPDIR)/lsp-framework-1.3.0 && mkdir -p build-debug && cd build-debug && CC=clang CXX=clang++ cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$(PREFIX)
	cd $(TMPDIR)/lsp-framework-1.3.0/build-debug && make
	@echo "Installing lsp-framework (Debug)..."
	cd $(TMPDIR)/lsp-framework-1.3.0/build-debug && cmake --install . --prefix $(TMPDIR)/lsp-framework-1.3.0/build-debug
	$(SUDO) install -Dm755 $(TMPDIR)/lsp-framework-1.3.0/build-debug/liblspd.a $(LIBDIR)/liblspd.a
	@echo "Copying lsp headers..."
	$(SUDO) mkdir -p $(INCLUDEDIR)/lsp
	$(SUDO) cp -r $(TMPDIR)/lsp-framework-1.3.0/build/include/lsp/ $(INCLUDEDIR)/
	@echo "Installing lsp-framework cmake config files..."
	$(SUDO) mkdir -p $(LIBDIR)/cmake/lsp
	$(SUDO) cp $(TMPDIR)/lsp-framework-1.3.0/build/lib/cmake/lsp/lspConfig.cmake $(LIBDIR)/cmake/lsp/
	$(SUDO) cp $(TMPDIR)/lsp-framework-1.3.0/build/lib/cmake/lsp/lspConfigVersion.cmake $(LIBDIR)/cmake/lsp/
	$(SUDO) cp $(TMPDIR)/lsp-framework-1.3.0/build/lib/cmake/lsp/lspConfigTargets.cmake $(LIBDIR)/cmake/lsp/
	@echo "✓ lsp-framework (Release & Debug) installed"

install-deps: install-vcpkg-deps install-core install-lsp-framework

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
	@echo "  make install-lsp-framework - Install lsp-framework dependency"
	@echo "  make install-core       - Install falcon_core"
	@echo "  make install-deps       - Installs all dependencies in order"
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
