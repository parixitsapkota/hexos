# --- Directories & Outputs ---
BUILD       := $(ARTIFACTS)/boot/mbr
BOOT512_BIN := $(ARTIFACTS)/boot512.bin

# --- Build Rules ---
all: prepare $(BOOT512_BIN)

prepare:
	@mkdir -p $(BUILD)

# Build Legacy MBR Bootloader
$(BOOT512_BIN): boot512.asm
	@echo "[+] $(COLOR_GREEN)Assembling $<...$(COLOR_RESET)"
	@$(ACC) -f bin $< -o $@
