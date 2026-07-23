# --- Colors ---
COLOR_RESET  := \033[0m
COLOR_RED    := \033[1;31m
COLOR_GREEN  := \033[1;32m
COLOR_YELLOW := \033[1;33m
COLOR_BLUE   := \033[1;34m

export COLOR_RESET COLOR_RED COLOR_GREEN COLOR_YELLOW COLOR_BLUE

# --- Path ---
ROOT_DIR  := $(shell pwd)
ARTIFACTS := $(ROOT_DIR)/build
DISK      := $(ARTIFACTS)/disk.img
# --- Sub-Dir paths ---
BOOT512_BIN := $(ARTIFACTS)/boot512.bin
UEFI_APP    := $(ARTIFACTS)/BOOTX64.EFI

# --- Config ---
CC          := clang
CFLAGS      := -ffreestanding -fno-pic -fno-asynchronous-unwind-tables -nostdlib \
               -I$(ROOT_DIR)/src/include -Wall -Wextra -Werror -O3
CFLAGS32    := $(CFLAGS) -target i386-pc-none-elf -m32 -march=i386
CFLAGS64    := $(CFLAGS) -target x86_64-pc-none-elf -m64 -march=x86-64 -mcmodel=kernel -mno-red-zone

export CC CFLAGS32 CFLAGS64 ARTIFACTS

# --- Sub-Dirs ---
SUBDIRS := src/bootloader

# --- Primary Targets ---
all: build_components $(DISK)
	@echo "$(COLOR_RED)[#] Build complete!$(COLOR_RESET)"

build_components:
	@echo "$(COLOR_YELLOW)[#] Building component: $(SUBDIRS)$(COLOR_RESET)"
	@$(MAKE) --no-print-directory -C $(SUBDIRS)

prepare_build:
	@mkdir -p $(ARTIFACTS)

$(DISK): $(BOOT512_BIN) $(UEFI_APP)
	@echo "[+] $(COLOR_GREEN)Creating raw disk image...$(COLOR_RESET)"
	@dd if=/dev/zero of=$(DISK) bs=512 count=90001 status=none

	@echo "[+] $(COLOR_GREEN)Partitioning disk (MBR / EFI System)...$(COLOR_RESET)"
	@echo 'label: dos' | sfdisk $(DISK) > /dev/null 2>&1
	@echo '2048,,ef,*' | sfdisk --append $(DISK) > /dev/null 2>&1

	@echo "[+] $(COLOR_GREEN)Injecting Legacy MBR bootloader...$(COLOR_RESET)"
	@dd if=$(BOOT512_BIN) of=$(DISK) bs=446 count=1 conv=notrunc status=none

	@echo "[+] $(COLOR_GREEN)Formatting partition via loop device...$(COLOR_RESET)"
	@set -e; \
	MNT_DIR=$$(mktemp -d); \
	LOOP_DEV=$$(sudo losetup -Pf --show $(DISK)); \
	trap 'sudo umount $$MNT_DIR 2>/dev/null || true; \
	      sudo rmdir $$MNT_DIR 2>/dev/null || true; \
	      sudo losetup -d $$LOOP_DEV 2>/dev/null || true' EXIT; \
	sudo mkfs.vfat -F 32 -n "HEXBOOT" $${LOOP_DEV}p1 > /dev/null; \
	sudo mount $${LOOP_DEV}p1 $$MNT_DIR; \
	sudo mkdir -p $$MNT_DIR/EFI/BOOT; \
	sudo cp $(UEFI_APP) $$MNT_DIR/EFI/BOOT/BOOTX64.EFI

run: all
	@echo "$(COLOR_YELLOW)[!] Starting QEMU (UEFI)...$(COLOR_RESET)"
	@mkdir -p $(ARTIFACTS)/qemu
	@qemu-system-x86_64 -machine q35 -m 1G \
		-drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/x64/OVMF_CODE.fd \
		-drive file=$(DISK),format=raw

clean:
	@echo "$(COLOR_YELLOW)[#] Cleaning global build artifacts...$(COLOR_RESET)"
	@rm -rf $(ARTIFACTS)/*

.PHONY: all build_components prepare_build run clean
