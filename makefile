# --- Colors ---
COLOR_RESET  := \033[0m
COLOR_RED    := \033[1;31m
COLOR_GREEN  := \033[1;32m
COLOR_YELLOW := \033[1;33m
COLOR_BLUE   := \033[1;34m

# --- Path ---
ROOT_DIR  := $(shell pwd)
BUILD_DIR := $(ROOT_DIR)/build
DISK      := $(BUILD_DIR)/disk.img

# --- Config ---
CC          := clang
CFLAGS := -ffreestanding -fno-pic -fno-asynchronous-unwind-tables \
		  -I$(ROOT_DIR)/src/include -Wall -Wextra -Werror -O3
CFLAGS32    := $(CFLAGS) -target i386-pc-none-elf -m32 -march=i386
CFLAGS64    := $(CFLAGS) -target x86_64-pc-none-elf -m64 -march=x86-64 -mcmodel=kernel -mno-red-zone

# --- export ---
export COLOR_RESET
export COLOR_RED
export COLOR_GREEN
export COLOR_YELLOW
export COLOR_BLUE
export CC
export CFLAGS
export CFLAGS32
export CFLAGS64
export BUILD_DIR
export DISK

# --- Sub-Dirs ---
SUBDIRS := src/bootloader

# --- Sub-Dir paths ---
BOOT512_BIN := $(BUILD_DIR)/boot512.bin
BOOT_BIN    := $(BUILD_DIR)/boot.bin
KERNEL_BIN    := $(BUILD_DIR)/kernel.bin

# --- Build ---
all: prepare_build $(SUBDIRS)
	@echo "$(COLOR_YELLOW)[#] Build complete!$(COLOR_RESET)"

prepare_build:
	@mkdir -p $(BUILD_DIR)

$(SUBDIRS):
	@echo "$(COLOR_YELLOW)[#] Building component: $@$(COLOR_RESET)"
	@mkdir -p $@
	@$(MAKE) --no-print-directory -C $@

$(DISK): all
	@echo "$(COLOR_BLUE)[#] Making disk image...$(COLOR_RESET)"
	@dd if=/dev/zero of=$(DISK) bs=512 count=90001 status=none
	@echo "$(COLOR_BLUE)[#] Writing boot512.bin disk image...$(COLOR_RESET)"
	@dd if=$(BOOT512_BIN) of=$(DISK) bs=512 count=1 conv=notrunc status=none
	@echo "$(COLOR_BLUE)[#] Writing boot.bin disk image...$(COLOR_RESET)"
	@dd if=$(BOOT_BIN) of=$(DISK) bs=512 seek=1 conv=notrunc status=none
	@echo "$(COLOR_BLUE)[#] Adding kernel.bin disk image...$(COLOR_RESET)"
	@printf "n\np\n1\n2048\n\nt\nc\na\nw\n" | fdisk $(DISK) > /dev/null 2>&1
	@echo "drive p: file=\"$(DISK)\" offset=1048576" > .temp
	@MTOOLSRC=.temp mformat -F -v "BOOT32" p:
# 	@MTOOLSRC=.temp mcopy $(KERNEL_BIN) p:
	@rm .temp

run: $(DISK)
	@echo "$(COLOR_YELLOW)[!] Starting QEMU...$(COLOR_RESET)"
	@mkdir -p $(BUILD_DIR)/qemu
	@qemu-system-x86_64 -machine pc -m 1G \
		-drive file=$(DISK),format=raw,index=0,media=disk

debug:
	bochs -f bochs_config -q

clean:
	@echo "$(COLOR_YELLOW)[#] Cleaning global build artifacts...$(COLOR_RESET)"
	@rm -rf $(BUILD_DIR)

.PHONY: all prepare_build $(SUBDIRS) clean
