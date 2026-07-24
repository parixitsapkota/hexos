# --- Directories & Outputs ---
BUILD       := $(ARTIFACTS)/boot/uefi
UEFI_APP    := $(ARTIFACTS)/BOOTX64.EFI
POSIX_UEFI  := $(ARTIFACTS)/uefi

# --- Config ---
UEFI64_CFLAGS := -ffreestanding -fno-pic -Wall -Wextra -Werror -O3 \
                 -target x86_64-unknown-windows-msvc -mno-red-zone \
                 -Wno-null-pointer-subtraction -D_STDINT_H -I$(POSIX_UEFI)/uefi

# --- C Source & Object Discovery ---
C_SRC   := $(wildcard *.c)
ASM_SRC := $(wildcard *.asm)
C_OBJ   := $(patsubst %.c, $(BUILD)/c/%.o, $(C_SRC))
ASM_OBJ := $(patsubst %.asm, $(BUILD)/asm/%.o, $(ASM_SRC))

OBJ     := $(C_OBJ) $(ASM_OBJ)

# --- Build Rules ---
.PHONY: all prepare

all: prepare $(UEFI_APP)

# Download and compile POSIX-UEFI dependency
$(ARTIFACTS)/uefi:
	@echo "$(COLOR_BLUE)[#] Downloading POSIX-UEFI dependency...$(COLOR_RESET)"
	@git clone --depth=1 https://gitlab.com/bztsrc/posix-uefi.git $(ARTIFACTS)/uefi > /dev/null 2>&1
	@echo "$(COLOR_BLUE)[#] Compiling POSIX-UEFI dependency...$(COLOR_RESET)"
	@$(MAKE) --no-print-directory -C $@/uefi > /dev/null 2>&1

prepare:
	@mkdir -p $(BUILD)/c
	@mkdir -p $(BUILD)/asm

# --- Linking Step ---
$(UEFI_APP): $(ARTIFACTS)/uefi $(OBJ)
	@echo "$(COLOR_RED)[#] Linking UEFI APP...$(COLOR_RESET)"
	@lld-link /subsystem:efi_application /entry:uefi_init /nodefaultlib \
	  /out:$@ $(POSIX_UEFI)/uefi/crt_x86_64.o \
	  $(OBJ) $(POSIX_UEFI)/uefi/libuefi.a

# --- C Compilation ---
$(BUILD)/c/%.o: %.c
	@echo "[+] $(COLOR_GREEN)Compiling $<...$(COLOR_RESET)"
	@$(CC) $(UEFI64_CFLAGS) -I../../include -I$(POSIX_UEFI)/uefi -c $< -o $@

# --- ASM Compilation ---
$(BUILD)/asm/%.o: %.asm
	@echo "[+] $(COLOR_GREEN)Assembling $<...$(COLOR_RESET)"
	@$(ACC) -f win64 $< -o $@