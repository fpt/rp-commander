# ardu-commander Makefile

.PHONY: help build clean deploy monitor shell docker-build rebuild info icons

.DEFAULT_GOAL := help

DOCKER_IMAGE := ardu-commander
BUILD_DIR    := build
UF2_FILE     := $(BUILD_DIR)/commander.uf2
PICO_SDK     := ../pico-sdk

MOUNT_POINT_MACOS := $(shell ls -d /Volumes/RP* 2>/dev/null | head -1)
MOUNT_POINT_LINUX := $(shell ls -d /media/*/RPI-RP2 2>/dev/null | head -1)
MOUNT_POINT := $(if $(MOUNT_POINT_MACOS),$(MOUNT_POINT_MACOS),$(MOUNT_POINT_LINUX))

SERIAL_PORT_MACOS := $(shell ls /dev/cu.usbmodem* 2>/dev/null | head -1)
SERIAL_PORT_LINUX := $(shell ls /dev/ttyACM* 2>/dev/null | head -1)
SERIAL_PORT := $(if $(SERIAL_PORT_MACOS),$(SERIAL_PORT_MACOS),$(SERIAL_PORT_LINUX))

COLOR_RESET  := \033[0m
COLOR_BOLD   := \033[1m
COLOR_GREEN  := \033[32m
COLOR_YELLOW := \033[33m
COLOR_BLUE   := \033[34m

define do_build
	@if ! docker images | grep -q $(DOCKER_IMAGE); then \
		echo "$(COLOR_YELLOW)Docker image not found. Building...$(COLOR_RESET)"; \
		$(MAKE) docker-build; \
	fi
	docker run --rm \
		-v "$$(pwd):/work" \
		-v "$$(cd $(PICO_SDK) && pwd):/pico-sdk" \
		-w /work \
		$(DOCKER_IMAGE) \
		/bin/bash -c " \
			set -e; \
			mkdir -p $(BUILD_DIR); \
			cd $(BUILD_DIR); \
			cmake -DPICO_SDK_PATH=/pico-sdk $(1) ..; \
			make -j$$(nproc); \
		"
endef

##@ General

help: ## Display this help
	@echo "$(COLOR_BOLD)ardu-commander - USB HID Commander$(COLOR_RESET)"
	@echo ""
	@awk 'BEGIN {FS = ":.*##"} /^[a-zA-Z_-]+:.*?##/ { printf "  $(COLOR_BLUE)%-15s$(COLOR_RESET) %s\n", $$1, $$2 } /^##@/ { printf "\n$(COLOR_BOLD)%s$(COLOR_RESET)\n", substr($$0, 5) }' $(MAKEFILE_LIST)
	@echo ""

info: ## Show project info
	@echo "$(COLOR_BOLD)Project Information:$(COLOR_RESET)"
	@echo "  Firmware:    $(UF2_FILE)"
	@echo "  Mount point: $(if $(MOUNT_POINT),$(COLOR_GREEN)$(MOUNT_POINT)$(COLOR_RESET),$(COLOR_YELLOW)Not detected$(COLOR_RESET))"
	@echo "  Serial port: $(if $(SERIAL_PORT),$(COLOR_GREEN)$(SERIAL_PORT)$(COLOR_RESET),$(COLOR_YELLOW)Not detected$(COLOR_RESET))"
	@if [ -f "$(UF2_FILE)" ]; then \
		echo "  Build status: $(COLOR_GREEN)Built$(COLOR_RESET) ($$(ls -lh $(UF2_FILE) | awk '{print $$5}'))"; \
	else \
		echo "  Build status: $(COLOR_YELLOW)Not built$(COLOR_RESET)"; \
	fi

##@ Build

docker-build: ## Build Docker image (only needed once)
	docker build -t $(DOCKER_IMAGE) .

build: ## Build firmware
	@echo "$(COLOR_BOLD)Building commander...$(COLOR_RESET)"
	$(call do_build,)
	@echo "$(COLOR_GREEN)Build complete!$(COLOR_RESET)"
	@ls -lh $(UF2_FILE)

rebuild: clean build ## Clean and rebuild

clean: ## Remove build artefacts
	rm -rf $(BUILD_DIR)
	@echo "$(COLOR_GREEN)Clean$(COLOR_RESET)"

##@ Deployment

deploy: build ## Upload firmware to RP2040 (hold BOOTSEL, connect USB, release)
	@if [ ! -f "$(UF2_FILE)" ]; then $(MAKE) build; fi
	@if [ -z "$(MOUNT_POINT)" ]; then \
		echo "$(COLOR_YELLOW)RP2040 not detected. Hold BOOTSEL then connect USB.$(COLOR_RESET)"; \
		exit 1; \
	fi
	cp $(UF2_FILE) $(MOUNT_POINT)/
	@if [ "$$(uname)" = "Darwin" ]; then diskutil eject $(MOUNT_POINT) 2>/dev/null || true; fi
	@echo "$(COLOR_GREEN)Deployed!$(COLOR_RESET)"

flash: deploy ## Alias for deploy

##@ Icons

icons: ## Generate src/icons_data.h from resources/ (requires: pip3 install Pillow cairosvg)
	python3 tools/make_icons.py

##@ Development

monitor: ## Connect to UART console (GP0=TX, GP1=RX via USB-serial adapter)
	@if [ -z "$(SERIAL_PORT)" ]; then \
		echo "$(COLOR_YELLOW)Serial port not detected$(COLOR_RESET)"; exit 1; \
	fi
	screen $(SERIAL_PORT) 115200

shell: ## Open shell in Docker container
	@if ! docker images | grep -q $(DOCKER_IMAGE); then $(MAKE) docker-build; fi
	docker run --rm -it \
		-v "$$(pwd):/work" \
		-v "$$(cd $(PICO_SDK) && pwd):/pico-sdk" \
		-w /work $(DOCKER_IMAGE) /bin/bash

##@ Quick Workflows

quick: build deploy ## Build and deploy
all: build deploy monitor ## Build, deploy, and monitor

watch: ## Rebuild on source change (requires entr: brew install entr)
	@find src -name "*.c" -o -name "*.h" | entr -r make quick
