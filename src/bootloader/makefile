# --- Path & Output ---
BUILD       := $(BUILD_DIR)/boot
BOOT512_BIN := $(BUILD_DIR)/boot512.bin
BOOT_BIN    := $(BUILD_DIR)/boot.bin
MAIN_ASM_O  := $(BUILD)/bmain.o

# --- C Source & Object Discovery ---
C_SRC     := $(wildcard *.c)
OBJ       := $(patsubst %.c, $(BUILD)/%.o, $(C_SRC))

# --- Build ---
all: prepare $(BOOT512_BIN) $(BOOT_BIN)

prepare:
	@mkdir -p $(BUILD)

# --- Linking ---
$(BOOT_BIN): $(OBJ) $ $(MAIN_ASM_O) linker.ld
	@echo "$(COLOR_RED)[#] Linking boot.bin...$(COLOR_RESET)"
	@ld.lld -T linker.ld $(OBJ) $(MAIN_ASM_O) -o $(BUILD)/boot.elf
	@llvm-objcopy -O binary $(BUILD)/boot.elf $(BOOT_BIN)

# --- Compilation ---
$(BUILD)/%.o: %.c
	@echo "[+] $(COLOR_GREEN)Compiling $< ...$(COLOR_RESET)"
	@$(CC) $(CFLAGS32) -c $< -o $@

$(BOOT512_BIN): boot512.asm
	@echo "[+] $(COLOR_GREEN)Compiling $< ...$(COLOR_RESET)"
	@nasm -f bin $< -o $@

$(MAIN_ASM_O): main.asm
	@echo "[+] $(COLOR_GREEN)Compiling $< ...$(COLOR_RESET)"
	@nasm -f elf32 $< -o $@

.PHONY: all
