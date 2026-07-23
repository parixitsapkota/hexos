# --- Directories & Outputs ---
BUILD       := $(ARTIFACTS)/boot/uefi
UEFI_APP    := $(ARTIFACTS)/BOOTX64.EFI
POSIX_UEFI  := $(ARTIFACTS)/uefi

# --- Config ---
UEFI64_CFLAGS := -ffreestanding -fno-pic -Wall -Wextra -Werror -O3 \
                 -target x86_64-unknown-windows-msvc -mno-red-zone \
                 -Wno-null-pointer-subtraction -I$(POSIX_UEFI)/uefi

# --- C Source & Object Discovery ---
C_SRC     := $(wildcard *.c)
OBJ       := $(patsubst %.c, $(BUILD)/%.o, $(C_SRC))

# --- Build Rules ---
all: prepare $(UEFI_APP)

# Download and compile POSIX-UEFI dependency
$(ARTIFACTS)/uefi:
	@echo "$(COLOR_BLUE)[#] Downloading POSIX-UEFI dependency...$(COLOR_RESET)"
	@git clone --depth=1 https://gitlab.com/bztsrc/posix-uefi.git $(ARTIFACTS)/uefi > /dev/null 2>&1
	@echo "$(COLOR_BLUE)[#] Compile POSIX-UEFI dependency...$(COLOR_RESET)"
	@$(MAKE) --no-print-directory -C $@/uefi CFLAGS+="-I. -Wno-null-pointer-subtraction -target x86_64-pc-win32-coff" > /dev/null 2>&1

prepare:
	@mkdir -p $(BUILD)

# --- Linking Step ---
$(UEFI_APP): $(ARTIFACTS)/uefi $(OBJ)
	@echo "$(COLOR_RED)[#] Linking UEFI APP...$(COLOR_RESET)"
	@lld-link -subsystem:efi_application -entry:uefi_init -nodefaultlib \
		-out:$@ $(POSIX_UEFI)/uefi/crt_x86_64.o \
		$(OBJ) $(POSIX_UEFI)/uefi/libuefi.a

# --- Compilation ---
$(BUILD)/%.o: %.c
	@echo "[+] $(COLOR_GREEN)Compiling $<...$(COLOR_RESET)"
	@clang $(UEFI64_CFLAGS) -I$(POSIX_UEFI)/uefi -c $< -o $@
