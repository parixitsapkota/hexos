#include <stdint.h>

#define PACKED __attribute__((packed))

typedef struct PACKED {
  uint32_t *g_framebuffer;
  uint16_t g_pitch;
  uint16_t g_height;
  uint16_t g_width;
  uint8_t g_bpp;
} FramebufferInfo;

typedef struct PACKED {
  uintptr_t map;
} MmapInfo;

typedef struct PACKED {
  uintptr_t map;
} CpuCoreInfo;

typedef struct PACKED {
  uintptr_t map;
} BootModules;

typedef struct PACKED {
  uint64_t g_hhdm_offset;
  FramebufferInfo g_fb_info;
  MmapInfo *g_mmap_info;
  uint64_t g_mmap_entries_count;
  uint64_t g_rsdp_address;
  CpuCoreInfo *g_cpus;
  BootModules *g_modules;
  char *cmdline;
} BootInfo;
