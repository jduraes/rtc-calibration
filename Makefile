SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c
.ONESHELL:
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules

# Compiler setup for RC2014/CP/M
Z88DK_PATH := /home/miguel/yellow-msx-series-for-rc2014/z88dk
PATH := $(Z88DK_PATH)/bin:$(PATH)
export PATH
ZCC := zcc +cpm -compiler=sdcc -create-app

# Application name (8.3 format)
APP := rtccalib

# Source files
SOURCES := rtccalib.c rp5c01.asm rp5c01.h cpm.asm cpm.h

all: $(APP).com

$(APP).com: $(SOURCES)
	$(ZCC) rtccalib.c rp5c01.asm cpm.asm -o $(APP).com
	@echo "Compiled $(APP).com successfully"

clean:
	rm -f *.com *.o *.map *.lst *.sym
	@echo "Cleaned build files"

install: $(APP).com
	@if [ -d "$(ROMWBW_APPS)" ]; then \
		cp $(APP).com "$(ROMWBW_APPS)/"; \
		echo "Installed $(APP).com to $(ROMWBW_APPS)"; \
	else \
		echo "ROMWBW_APPS directory not set or doesn't exist"; \
		echo "Please copy $(APP).com to your RC2014 system manually"; \
	fi

test: $(APP).com
	@echo "Built $(APP).com for RC2014"
	@echo "Copy to your RC2014 system and run under CP/M"

help:
	@echo "RTC Calibration Utility Build System"
	@echo "===================================="
	@echo "Targets:"
	@echo "  all      - Build $(APP).com (default)"
	@echo "  clean    - Remove build files"
	@echo "  install  - Copy to ROMWBW_APPS directory"
	@echo "  test     - Build and show usage info"
	@echo "  help     - Show this help"
	@echo ""
	@echo "Requirements:"
	@echo "  - z88dk compiler suite"
	@echo "  - Make"
	@echo ""
	@echo "For RC2014 with Ed Brindley's RTC Card Rev1"

.PHONY: all clean install test help
