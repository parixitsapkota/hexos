#ifndef _KERNEL_H_
#define _KERNEL_H_

#include <stdint.h>

#define PACKED __attribute__((packed))

typedef struct PACKED {
  uint32_t *g_framebuffer; // Pointer to linear video frame buffer
  uint16_t g_pitch;        // Bytes per scanline
  uint16_t g_height;       // Vertical resolution in pixels
  uint16_t g_width;        // Horizontal resolution in pixels
  uint8_t g_bpp;           // Bits per pixel
} FramebufferInfo;

typedef enum : uint32_t {
  KMM_USABLE = 1,
  KMM_UNUSABLE = 2,
  KMM_RESERVED = 3,
  KMM_ACPI_RECLAIMABLE = 4,
} MemMapKind;

typedef struct PACKED {
  uint64_t base;   // Physical base address of memory region
  uint64_t length; // Size of the memory region in bytes
  MemMapKind kind; // Usage classification
} MemMap;

typedef struct PACKED {
  uint32_t lapic_id; // Local APIC / Core ID
} CpuCore;

typedef struct PACKED {
  uint64_t base; // start address of module
  uint64_t size; // Size of the module in bytes
  char *name;    // Null-terminated module name
} BootModules;

typedef struct PACKED {
  uint64_t g_hhdm_offset;  // Higher-Half Direct Map virtual offset
  uint64_t g_rsdp_address; // Physical address of ACPI RSDP table

  FramebufferInfo g_fb_info; // Direct embedded framebuffer metadata

  uint64_t g_mmap_entries_count; // Number of entries in memory map
  MemMap *g_mmap_info;           // MmapInfo

  uint32_t g_cpu_count; // Number of detected CPUs
  CpuCore *g_cpus;      // CpuCore structs

  uint32_t g_module_count; // Number of loaded modules
  BootModules *g_modules;  // BootModules
} BootInfo;

typedef void (*KernelEntry)(BootInfo* boot_info);

#endif // _KERNEL_H_
