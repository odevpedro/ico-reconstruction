# ICO splat build pipeline
#
# Prerequisites:
#   - Python venv with splat64[mips] >= 0.34.0
#   - EE GCC 2.9-991111-01 (Sony R5900 toolchain)
#     The ICO binary was compiled with:
#       -march=r5900 -mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2
#
# Without the matching compiler, the 'build' target will not produce a
# byte-identical ELF. 'make split' works with any splat installation.

SPLAT := /tmp/ico-tooling-venv/bin/splat
CONFIG := SCUS_971.13.cloth-full.yaml
BASENAME := SCUS_971.13

# Output directories (generated asm stays outside the repo)
BUILD_BASE := /tmp/ico-cloth-full
ASM := $(BUILD_BASE)/asm
BUILD := $(BUILD_BASE)/build
ASSETS := $(BUILD_BASE)/assets

# EE GCC toolchain (point these to your ee-gcc installation)
CC := ee-gcc
AS := ee-as
LD := ee-ld
OBJCOPY := ee-objcopy

# Compiler flags matching ICO USA SCUS_971.13
CFLAGS := -march=r5900 -mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2

.PHONY: all split build clean

all: split

split:
	$(SPLAT) split $(CONFIG)

build: split
	$(MAKE) -C $(BUILD) -f ../$(BASENAME).ld

clean:
	rm -rf $(BUILD_BASE)

# first-diff: compare sha1 of rebuilt ELF vs original
.PHONY: check
check: build
	@echo "=== Checking rebuilt ELF ==="
	@sha1sum $(BUILD_BASE)/$(BASENAME).elf 2>/dev/null || \
	 echo "Rebuilt ELF not found. Verify ee-gcc toolchain path in Makefile."
	@echo "Expected: a4d8fc1948fb2da2395f3863a94a3b93de55de14"
