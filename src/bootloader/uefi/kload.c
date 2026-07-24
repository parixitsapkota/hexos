#include <stdint.h>

#ifndef _STDINT_H
#define _STDINT_H
#endif

#include <uefi.h>

void *load_kernel_file(uint64_t target_phys_addr, const char *filename) {
  FILE *f = fopen(filename, "r");
  if (!f) {
    printf("Failed to open file: %s\n", filename);
    return NULL;
  }

  fseek(f, 0, SEEK_END);
  uint64_t size = ftell(f);
  fseek(f, 0, SEEK_SET);

  uintn_t pages = (size + 4095) / 4096;
  efi_physical_address_t phys_addr = target_phys_addr;

  // Try EXACT target address first
  efi_status_t status = BS->AllocatePages(AllocateAddress, EfiLoaderData, pages, &phys_addr);

  // Fallback: If exact address is blocked, let UEFI pick anywhere below 4GB
  if (EFI_ERROR(status)) {
    printf("Exact address 0x%lx unavailable (%r).\n"
           "Falling back to dynamic allocation...\n",
           target_phys_addr, status);

    phys_addr = 0xFFFFFFFF; // Upper limit 4GB
    status = BS->AllocatePages(AllocateMaxAddress, EfiLoaderData, pages, &phys_addr);
  }

  if (EFI_ERROR(status)) {
    printf("Failed to allocate %lu pages for kernel: %r\n", pages, status);
    fclose(f);
    return NULL;
  }

  size_t bytes_read = fread((void *)phys_addr, 1, size, f);
  fclose(f);

  if (bytes_read != size) {
    printf("Warning: Expected %llu bytes, read %zu bytes\n", size, bytes_read);
    return 0;
  }

  return (void *)phys_addr;
}
