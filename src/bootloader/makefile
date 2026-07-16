# --- Path & Output ---
BUILD       := $(BUILD_DIR)/boot
BOOT512_BIN := $(BUILD_DIR)/boot512.bin
BOOT_BIN    := $(BUILD_DIR)/boot.bin

# --- C Source & Object Discovery ---
C_SRC     := $(wildcard stage2/*.c)
ASM_SRC   := $(wildcard stage2/*.asm)

# Strip the "stage2/" prefix here so objects are placed directly in $(BUILD)/c/ and $(BUILD)/asm/
C_OBJ     := $(patsubst stage2/%.c, $(BUILD)/c/%.o, $(C_SRC))
ASM_OBJ   := $(patsubst stage2/%.asm, $(BUILD)/asm/%.o, $(ASM_SRC))

# --- Build ---
all: prepare $(BOOT512_BIN) $(BOOT_BIN)

prepare:
	@mkdir -p $(BUILD)
	@mkdir -p $(BUILD)/c
	@mkdir -p $(BUILD)/asm

# --- Linking (Now properly depends on ASM_OBJ) ---
$(BOOT_BIN): $(C_OBJ) $(ASM_OBJ) stage2/linker.ld
	@echo "$(COLOR_RED)[#] Linking boot.bin...$(COLOR_RESET)"
	@ld.lld -T stage2/linker.ld $(C_OBJ) $(ASM_OBJ) -o $(BUILD)/boot.elf
	@llvm-objcopy -O binary $(BUILD)/boot.elf $(BOOT_BIN)

# --- Compilation ---
$(BUILD)/c/%.o: stage2/%.c
	@echo "[+] $(COLOR_GREEN)Compiling $< ...$(COLOR_RESET)"
	@$(CC) $(CFLAGS32) -c $< -o $@

$(BOOT512_BIN): stage1/boot512.asm
	@echo "[+] $(COLOR_GREEN)Compiling $< ...$(COLOR_RESET)"
	@nasm -f bin $< -o $@

$(BUILD)/asm/%.o: stage2/%.asm
	@echo "[+] $(COLOR_GREEN)Compiling $< ...$(COLOR_RESET)"
	@nasm -f elf32 $< -o $@

.PHONY: all
