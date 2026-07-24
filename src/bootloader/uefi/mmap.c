#include <stdint.h>

#ifndef _STDINT_H
#define _STDINT_H
#endif

#include <kernel.h>
#include <uefi.h>

extern BootInfo g_boot_info;
extern efi_status_t status;

static efi_guid_t loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;

void get_bootloader_memory_range(uint64_t *base_addr, uint64_t *size) {
  efi_loaded_image_protocol_t *loaded_image = NULL;

  efi_status_t status = BS->HandleProtocol(IM, &loaded_image_guid, (void **)&loaded_image);

  if (!EFI_ERROR(status) && loaded_image != NULL) {
    *base_addr = (uint64_t)loaded_image->ImageBase;
    *size = (uint64_t)loaded_image->ImageSize;
  } else {
    *base_addr = 0;
    *size = 0;
  }
}

void init_mmap(void) {
  uintn_t mmap_size = 0;
  efi_memory_descriptor_t *uefi_mmap = NULL;
  uintn_t map_key = 0;
  uintn_t desc_size = 0;
  uint32_t desc_version = 0;

  BS->GetMemoryMap(&mmap_size, NULL, &map_key, &desc_size, &desc_version);
  mmap_size += 8 * desc_size;

  uefi_mmap = malloc(mmap_size);
  status = BS->GetMemoryMap(&mmap_size, uefi_mmap, &map_key, &desc_size, &desc_version);

  if (!EFI_ERROR(status)) {
    uintn_t entry_count = mmap_size / desc_size;

    g_boot_info.g_mmap_info = malloc(entry_count * sizeof(MemMap));
    g_boot_info.g_mmap_entries_count = 0;

    uint8_t *ptr = (uint8_t *)uefi_mmap;
    for (uintn_t i = 0; i < entry_count; i++) {
      efi_memory_descriptor_t *desc = (efi_memory_descriptor_t *)(ptr + (i * desc_size));

      MemMap *info = &g_boot_info.g_mmap_info[g_boot_info.g_mmap_entries_count];
      info->base = desc->PhysicalStart;
      info->length = desc->NumberOfPages * 4096;

      switch (desc->Type) {
      case EfiConventionalMemory:
      case EfiLoaderCode:
      case EfiLoaderData:
      case EfiBootServicesCode:
      case EfiBootServicesData: info->kind = KMM_USABLE; break;
      case EfiACPIReclaimMemory: info->kind = KMM_ACPI_RECLAIMABLE; break;
      case EfiUnusableMemory: info->kind = KMM_UNUSABLE; break;
      default: info->kind = KMM_RESERVED; break;
      }
      g_boot_info.g_mmap_entries_count++;
    }
  }

  free(uefi_mmap);
}
