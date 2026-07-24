#include <stdint.h>

#ifndef _STDINT_H
#define _STDINT_H
#endif

#include <kernel.h>
#include <uefi.h>

void setup_paging(void);
void init_cpu_cores(void);
void init_mmap(void);
void *load_kernel_file(uint64_t phys_addr, const char *filename);
void get_bootloader_memory_range(uint64_t *base_addr, uint64_t *size);

BootInfo g_boot_info;
efi_status_t status;
static efi_guid_t gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
static efi_guid_t acpi_guid = ACPI_20_TABLE_GUID;

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  if (ST != NULL && ST->ConOut != NULL) {
    ST->ConOut->ClearScreen(ST->ConOut);
  }

  printf("Initializing BootInfo...\n");
  // Query Bootloader Physical Memory Range
  uint64_t bootloader_base = 0;
  uint64_t bootloader_size = 0;
  get_bootloader_memory_range(&bootloader_base, &bootloader_size);
  // uint64_t bootloader_pages = (bootloader_size + 4095) / 4096;

  // load Kernel to Specific Address
  uint64_t phys_addr = 0x02000000;
  phys_addr = (uint64_t)load_kernel_file(phys_addr, "\\EFI\\BOOT\\license");

  // Locate Framebuffer
  efi_gop_t *gop = NULL;
  status = BS->LocateProtocol(&gop_guid, NULL, (void **)&gop);

  if (!EFI_ERROR(status) && gop != NULL && gop->Mode != NULL && gop->Mode->Information != NULL) {
    g_boot_info.g_fb_info.g_framebuffer = (uint32_t *)gop->Mode->FrameBufferBase;

    g_boot_info.g_fb_info.g_width = (uint16_t)gop->Mode->Information->HorizontalResolution;
    g_boot_info.g_fb_info.g_height = (uint16_t)gop->Mode->Information->VerticalResolution;
    g_boot_info.g_fb_info.g_pitch =
        (uint16_t)(gop->Mode->Information->PixelsPerScanLine * sizeof(uint32_t));
    g_boot_info.g_fb_info.g_bpp = 32;
  }

  // Locate ACPI RSDP Pointer
  for (uintn_t i = 0; i < ST->NumberOfTableEntries; i++) {
    if (!memcmp(&ST->ConfigurationTable[i].VendorGuid, &acpi_guid, sizeof(efi_guid_t))) {
      g_boot_info.g_rsdp_address = (uint64_t)ST->ConfigurationTable[i].VendorTable;
      break;
    }
  }

  // Extract System Memory Map from UEFI Boot Services
  init_mmap();

  // CPU Initialization
  init_cpu_cores();

  // Boot Modules Initialization
  g_boot_info.g_module_count = 0;
  g_boot_info.g_modules = NULL;

  if (phys_addr == 0) {
    printf("Can't load kernel.\n");
    printf("\nPress [q] to shutdown\n");
    for (;;) {
      int c = getchar();

      if (c == 'q') {
        RT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      }
    }
  }

  printf("Loaded kernel at physical address: 0x%lx\n", phys_addr);
  printf("GOP Found: %dx%d @ 0x%lx\n", g_boot_info.g_fb_info.g_width,
         g_boot_info.g_fb_info.g_height, (uintptr_t)g_boot_info.g_fb_info.g_framebuffer);
  printf("Memory map populated (%d entries).\n", (int)g_boot_info.g_mmap_entries_count);
  printf("ACPI RSDP @ 0x%lx\n", g_boot_info.g_rsdp_address);
  printf("CPUs initialized: %u core(s)\n", g_boot_info.g_cpu_count);
  for (uint32_t i = 0; i < g_boot_info.g_cpu_count; i++) {
    printf("CPU-CORE-%d ID: %u\n", i + 1, g_boot_info.g_cpus[i].lapic_id);
  }
  printf("BootInfo setup complete!\n");
  printf("\nPress [q] to shutdown\n");
  for (;;) {
    int c = getchar();

    if (c == 'q') {
      RT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    }
  }

  return 0;
}
