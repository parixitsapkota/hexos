#ifndef _FAT32_H_
#define _FAT32_H_

#include <stdint.h>

#include "disk.h"

// FAT32 Boot Sector and BPB (512 bytes total)
typedef struct PACKED {
  // FAT12/16/32 Common Fields (Offset 0 to 35)
  uint8_t BS_JmpBoot[3];   // Jump instruction to bootstrap code
  uint8_t BS_OEMName[8];   // OEM Name string
  uint16_t BPB_BytsPerSec; // Bytes per sector (512)
  uint8_t BPB_SecPerClus;  // Sectors per allocation unit (Cluster)
  uint16_t BPB_RsvdSecCnt; // Number of reserved sectors (usually 32 for FAT32)
  uint8_t BPB_NumFATs;     // Number of FAT copies (usually 2)
  uint16_t BPB_RootEntCnt; // FAT12/16 root directory entries (Must be 0 for FAT32)
  uint16_t BPB_TotSec16;   // 16-bit total sector count (Must be 0 for FAT32)
  uint8_t BPB_Media;       // Media descriptor byte (e.g., 0xF8 for fixed disk)
  uint16_t BPB_FATSz16;    // 16-bit size of one FAT (Must be 0 for FAT32)
  uint16_t BPB_SecPerTrk;  // Sectors per track for geometry-based media
  uint16_t BPB_NumHeads;   // Number of heads for geometry-based media
  uint32_t BPB_HiddSec;    // Number of hidden physical sectors preceding volume
  uint32_t BPB_TotSec32;   // 32-bit total sector count (used when TotSec16 == 0)

  // FAT32 Specific Fields (Offset 36 to 89)
  uint32_t BPB_FATSz32;     // 32-bit size of one FAT in sectors
  uint16_t BPB_ExtFlags;    // Mirroring flags (Bits 0-3: Active FAT, Bit 7: No mirror)
  uint16_t BPB_FSVer;       // Filesystem version (High byte: Major, Low byte: Minor)
  uint32_t BPB_RootClus;    // Cluster number of the start of the root directory
  uint16_t BPB_FSInfo;      // Sector number of the FSInfo structure (usually 1)
  uint16_t BPB_BkBootSec;   // Sector number of the backup boot sector (usually 6)
  uint8_t BPB_Reserved[12]; // Reserved for future expansion

  uint8_t BS_DrvNum;        // IBM PC drive number (0x00 for floppy, 0x80 for fixed)
  uint8_t BS_Reserved1;     // Reserved (used by Windows NT)
  uint8_t BS_BootSig;       // Extended boot signature (0x29 indicates next 3 fields exist)
  uint32_t BS_VolID;        // Volume serial number
  uint8_t BS_VolLab[11];    // Volume label string
  uint8_t BS_FilSysType[8]; // Filesystem type string (e.g., "FAT32   ")

  // Bootstrap Code & Signature (Offset 90 to 511)
  uint8_t BS_BootCode[420]; // X86 Bootstrap program code
  uint16_t BS_BootSign;     // Boot sector signature (0xAA55)
} FAT32_BPB;

// FAT Standard Directory Entry (32 bytes)
typedef struct PACKED {
  uint8_t DIR_Name[11];     // Short File Name (8 chars Name + 3 chars Extension)
  uint8_t DIR_Attr;         // File attributes (Read-only, Hidden, System, Directory, etc.)
  uint8_t DIR_NTRes;        // Reserved for Windows NT (case-preservation flags)
  uint8_t DIR_CrtTimeTenth; // Millisecond component of creation time (0-199 in 10ms units)
  uint16_t DIR_CrtTime;     // File creation time (MS-DOS format)
  uint16_t DIR_CrtDate;     // File creation date (MS-DOS format)
  uint16_t DIR_LstAccDate;  // Last access date (MS-DOS format)
  uint16_t DIR_FstClusHI;   // High 16 bits of the file's first cluster number (FAT32 only)
  uint16_t DIR_WrtTime;     // Last write/modification time (MS-DOS format)
  uint16_t DIR_WrtDate;     // Last write/modification date (MS-DOS format)
  uint16_t DIR_FstClusLO;   // Low 16 bits of the file's first cluster number
  uint32_t DIR_FileSize;    // File size in bytes
} FAT_DirEntry;

uint32_t cluster_to_lba(FAT32_BPB *fat32, uint32_t cluster, uint32_t partition_lba);
uint32_t get_fat_entry_lba(FAT32_BPB *fat32, uint32_t cluster, uint32_t partition_lba);
uint32_t get_fat_entry_offset(FAT32_BPB *fat32, uint32_t cluster);
void to_dos_filename(const char *input, uint8_t *output);
uint8_t fat32_find_file_in_dir(Disk disk, FAT32_BPB *fat32, uint32_t partition_lba, uint32_t dir_cluster,
                          const char *target_filename, uint32_t *out_start_cluster,
                          uint32_t *out_file_size);
uint8_t *fat32_read_file(Disk disk, FAT32_BPB *fat32, uint32_t partition_lba, uint32_t start_cluster,
                         uint32_t file_size);

#endif // _FAT32_H_
