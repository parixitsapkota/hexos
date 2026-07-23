# --- Sub-Dirs ---
SUBDIRS := mbr uefi

.PHONY: all $(SUBDIRS)

# --- Build ---
all: $(SUBDIRS)
	@echo "$(COLOR_RED)[#] Bootloader build complete!$(COLOR_RESET)"

$(SUBDIRS):
	@echo "$(COLOR_YELLOW)[#] Building sub-component: $@$(COLOR_RESET)"
	@$(MAKE) --no-print-directory -C $@
