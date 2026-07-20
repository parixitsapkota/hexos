#include <kernel.h>

#include "bprintf.h"
#include "disk.h"
#include "fat32.h"
#include "flib.h"
#include "paging.h"

// Variables
// Framebuffer information
extern uint32_t g_framebuffer;
extern uint16_t g_pitch;
extern uint16_t g_height;
extern uint16_t g_width;
extern uint8_t g_bpp;

// Functions
extern void setup_vesa16(void);
extern void switch64(void);
extern void pmode64_kernel(BootInfo *info);

static volatile uint8_t *kernel_load_adr = (volatile uint8_t *)0x00100000;

void bmain(void) {
  // master drive (0xE0)
  Disk master_disk = 0xE0;

  void *mbrprt_buffer = falloc(512);
  lba_read(master_disk, 0, 1, mbrprt_buffer);
  PartitionTable *mbrprt = (PartitionTable *)(mbrprt_buffer + 446);

  uint8_t partition = 0;
  for (partition = 0; partition < 4; ++partition) {
    if (mbrprt[partition].partitions->bootable == 128) {
      break;
    }
  }

  uint32_t first_sector_lba = mbrprt[partition].partitions->first_sector_lba;
  FAT32_BPB *fat32 = falloc(512);
  lba_read(master_disk, first_sector_lba, 1, (uint8_t *)fat32);

  const char *search_file = "kernel.bin";
  uint32_t start_cluster = 0;
  uint32_t file_size = 0;

  bprintf("Searching for file: '%s' in Root Directory...\n", search_file);
  uint8_t fat_has_file =
      fat32_find_file_in_dir(master_disk, fat32, first_sector_lba, fat32->BPB_RootClus,
                             search_file, &start_cluster, &file_size);

  if (fat_has_file) {
    setup_vesa16();

    static BootInfo boot_info = {0};
    boot_info.g_fb_info.g_framebuffer = &g_framebuffer;
    boot_info.g_fb_info.g_pitch = g_pitch;
    boot_info.g_fb_info.g_height = g_height;
    boot_info.g_fb_info.g_width = g_width;
    boot_info.g_fb_info.g_bpp = g_bpp;

    uint8_t *file =
        fat32_read_file(master_disk, fat32, first_sector_lba, start_cluster, file_size);
    uint32_t file_words = (file_size + 3) / 4;
    wordcpy32((void *)kernel_load_adr, file, file_words);

    init_paging();
    setup_paging();
    bprintf("Loading Kernel64.\n", search_file);
    // switch64();
  } else {
    bprintf("Error: File not found. '%s'\n", search_file);
  }
}
