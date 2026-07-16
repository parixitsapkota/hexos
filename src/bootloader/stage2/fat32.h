#ifndef _FAT32_H_
#define _FAT32_H_

#include <int.h>

#include "disk.h"

// FAT32 Boot Sector and BPB (512 bytes total)
typedef struct PACKED {
  // FAT12/16/32 Common Fields (Offset 0 to 35)
  u8 BS_JmpBoot[3];   // Jump instruction to bootstrap code
  u8 BS_OEMName[8];   // OEM Name string
  u16 BPB_BytsPerSec; // Bytes per sector (512)
  u8 BPB_SecPerClus;  // Sectors per allocation unit (Cluster)
  u16 BPB_RsvdSecCnt; // Number of reserved sectors (usually 32 for FAT32)
  u8 BPB_NumFATs;     // Number of FAT copies (usually 2)
  u16 BPB_RootEntCnt; // FAT12/16 root directory entries (Must be 0 for FAT32)
  u16 BPB_TotSec16;   // 16-bit total sector count (Must be 0 for FAT32)
  u8 BPB_Media;       // Media descriptor byte (e.g., 0xF8 for fixed disk)
  u16 BPB_FATSz16;    // 16-bit size of one FAT (Must be 0 for FAT32)
  u16 BPB_SecPerTrk;  // Sectors per track for geometry-based media
  u16 BPB_NumHeads;   // Number of heads for geometry-based media
  u32 BPB_HiddSec;    // Number of hidden physical sectors preceding volume
  u32 BPB_TotSec32;   // 32-bit total sector count (used when TotSec16 == 0)

  // FAT32 Specific Fields (Offset 36 to 89)
  u32 BPB_FATSz32;     // 32-bit size of one FAT in sectors
  u16 BPB_ExtFlags;    // Mirroring flags (Bits 0-3: Active FAT, Bit 7: No mirror)
  u16 BPB_FSVer;       // Filesystem version (High byte: Major, Low byte: Minor)
  u32 BPB_RootClus;    // Cluster number of the start of the root directory
  u16 BPB_FSInfo;      // Sector number of the FSInfo structure (usually 1)
  u16 BPB_BkBootSec;   // Sector number of the backup boot sector (usually 6)
  u8 BPB_Reserved[12]; // Reserved for future expansion

  u8 BS_DrvNum;        // IBM PC drive number (0x00 for floppy, 0x80 for fixed)
  u8 BS_Reserved1;     // Reserved (used by Windows NT)
  u8 BS_BootSig;       // Extended boot signature (0x29 indicates next 3 fields exist)
  u32 BS_VolID;        // Volume serial number
  u8 BS_VolLab[11];    // Volume label string
  u8 BS_FilSysType[8]; // Filesystem type string (e.g., "FAT32   ")

  // Bootstrap Code & Signature (Offset 90 to 511)
  u8 BS_BootCode[420]; // X86 Bootstrap program code
  u16 BS_BootSign;     // Boot sector signature (0xAA55)
} FAT32_BPB;

// FAT Standard Directory Entry (32 bytes)
typedef struct PACKED {
  u8 DIR_Name[11];     // Short File Name (8 chars Name + 3 chars Extension)
  u8 DIR_Attr;         // File attributes (Read-only, Hidden, System, Directory, etc.)
  u8 DIR_NTRes;        // Reserved for Windows NT (case-preservation flags)
  u8 DIR_CrtTimeTenth; // Millisecond component of creation time (0-199 in 10ms units)
  u16 DIR_CrtTime;     // File creation time (MS-DOS format)
  u16 DIR_CrtDate;     // File creation date (MS-DOS format)
  u16 DIR_LstAccDate;  // Last access date (MS-DOS format)
  u16 DIR_FstClusHI;   // High 16 bits of the file's first cluster number (FAT32 only)
  u16 DIR_WrtTime;     // Last write/modification time (MS-DOS format)
  u16 DIR_WrtDate;     // Last write/modification date (MS-DOS format)
  u16 DIR_FstClusLO;   // Low 16 bits of the file's first cluster number
  u32 DIR_FileSize;    // File size in bytes
} FAT_DirEntry;

u32 cluster_to_lba(FAT32_BPB *fat32, u32 cluster, u32 partition_lba);
u32 get_fat_entry_lba(FAT32_BPB *fat32, u32 cluster, u32 partition_lba);
u32 get_fat_entry_offset(FAT32_BPB *fat32, u32 cluster);
void to_dos_filename(const char *input, u8 *output);
u8 fat32_find_file_in_dir(Disk disk, FAT32_BPB *fat32, u32 partition_lba, u32 dir_cluster,
                          const char *target_filename, u32 *out_start_cluster,
                          u32 *out_file_size);
u8 *fat32_read_file(Disk disk, FAT32_BPB *fat32, u32 partition_lba, u32 start_cluster,
                    u32 file_size);

#endif // _FAT32_H_
