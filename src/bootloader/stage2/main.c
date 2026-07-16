#include <kernel.h>

#include "disk.h"
#include "fat32.h"
#include "flib.h"
#include "paging.h"

extern void setup_vesa16(void);
extern void switch64(void);
extern void pmode64_kernel(BootInfo *info);

static volatile u8 *kernel_load_adr = (volatile u8 *)0x00100000;
static KernelEntry kernel;

void bmain(void) {
  kernel = (KernelEntry)kernel_load_adr;
  // master drive (0xE0)
  Disk master_disk = 0xE0;

  void *mbrprt_buffer = falloc(512);
  lba_read(master_disk, 0, 1, mbrprt_buffer);
  PartitionTable *mbrprt = (PartitionTable *)(mbrprt_buffer + 446);

  u8 partition = 0;
  for (partition = 0; partition < 4; ++partition) {
    if (mbrprt[partition].partitions->bootable == 128) {
      break;
    }
  }

  u32 first_sector_lba = mbrprt[partition].partitions->first_sector_lba;
  FAT32_BPB *fat32 = falloc(512);
  lba_read(master_disk, first_sector_lba, 1, (u8 *)fat32);

  const char *search_file = "kernel.bin";
  u32 start_cluster = 0;
  u32 file_size = 0;

  bprintf("Searching for file: '%s' in Root Directory...\n", search_file);
  u8 fat_has_file =
      fat32_find_file_in_dir(master_disk, fat32, first_sector_lba, fat32->BPB_RootClus,
                             search_file, &start_cluster, &file_size);

  if (fat_has_file) {
    // setup_vesa16();

    static BootInfo boot_info = {0};
    boot_info.g_framebuffer = g_framebuffer;
    boot_info.g_pitch = g_pitch;
    boot_info.g_height = g_height;
    boot_info.g_width = g_width;
    boot_info.g_bpp = g_bpp;

    u8 *file = fat32_read_file(master_disk, fat32, first_sector_lba, start_cluster, file_size);
    u32 file_words = (file_size + 3) / 4;
    wordcpy32((void *)kernel_load_adr, file, file_words);

    init_paging();
    setup_paging();
    bprintf("Loading Kernel64.\n", search_file);
    switch64();
  } else {
    bprintf("Error: File not found. '%s'\n", search_file);
  }
}
