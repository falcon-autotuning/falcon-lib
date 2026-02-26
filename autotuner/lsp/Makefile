# Falcon LSP Makefile

.PHONY: all configure-debug configure-release build-debug build-release test clean install help

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    CMAKE_GENERATOR := Ninja
    NPROC := $(shell nproc 2>/dev/null || echo 4)
    SUDO := sudo
endif

BUILD_DIR_DEBUG   := build/debug
BUILD_DIR_RELEASE := build/release
INSTALL_PREFIX    ?= /opt/falcon

LSP_DIR  := /opt/falcon/lib/cmake/lsp
GTEST_DIR := /usr/local/lib/cmake/GTest

all: build-release

help:
	@echo "Falcon LSP Build System"
	@echo "========================"
	@echo "  make build-debug    Build debug version with tests"
	@echo "  make build-release  Build optimised release version"
	@echo "  make test           Run all tests"
	@echo "  make install        Install falcon-lsp and falcon-lint"
	@echo "  make clean          Remove build artifacts"

configure-debug:
	@mkdir -p $(BUILD_DIR_DEBUG)
	cd $(BUILD_DIR_DEBUG) && cmake ../.. \
		-DCMAKE_BUILD_TYPE=Debug \
		-DBUILD_TESTS=ON \
		-DBUILD_TOOLS=ON \
		-Dlsp_DIR=$(LSP_DIR) \
		-DGTest_DIR=$(GTEST_DIR) \
		-G $(CMAKE_GENERATOR)

configure-release:
	@mkdir -p $(BUILD_DIR_RELEASE)
	cd $(BUILD_DIR_RELEASE) && cmake ../.. \
		-DCMAKE_BUILD_TYPE=Release \
		-DBUILD_TESTS=OFF \
		-DBUILD_TOOLS=ON \
		-Dlsp_DIR=$(LSP_DIR) \
		-DGTest_DIR=$(GTEST_DIR) \
		-G $(CMAKE_GENERATOR)

build-debug: configure-debug
	ninja -C $(BUILD_DIR_DEBUG) -j$(NPROC)

build-release: configure-release
	ninja -C $(BUILD_DIR_RELEASE) -j$(NPROC)

test: build-debug
	cd $(BUILD_DIR_DEBUG) && ctest --output-on-failure

install: build-release
	$(SUDO) cmake --install $(BUILD_DIR_RELEASE) --prefix $(INSTALL_PREFIX)

clean:
	rm -rf build/
