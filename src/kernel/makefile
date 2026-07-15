# --- Path & Output ---
BUILD         := $(BUILD_DIR)/kernel
KERNEL_BIN    := $(BUILD_DIR)/kernel.bin
MAIN_ASM_O    := $(BUILD_DIR)/mainasm.o
 
# --- C Source & Object Discovery ---
C_SRC     := $(wildcard *.c)
OBJ       := $(patsubst %.c, $(BUILD)/%.o, $(C_SRC))

# --- Build ---
all: prepare $(KERNEL_BIN)

prepare:
	@mkdir -p $(BUILD)

# --- Linking ---
$(KERNEL_BIN): $(OBJ) linker.ld
	@echo "$(COLOR_RED)[#] Linking kernel.bin...$(COLOR_RESET)"
	@ld.lld -m elf_x86_64 -T linker.ld $(OBJ) -o $(BUILD)/kernel.elf
	@llvm-objcopy -O binary $(BUILD)/kernel.elf $(KERNEL_BIN)

# --- Compilation ---
$(BUILD)/%.o: %.c
	@echo "[+] $(COLOR_GREEN)Compiling $< ...$(COLOR_RESET)"
	@$(CC) $(CFLAGS64) -c $< -o $@

.PHONY: all
