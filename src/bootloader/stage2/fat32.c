#include "fat32.h"
#include "disk.h"
#include "flib.h"

uint32_t cluster_to_lba(FAT32_BPB *fat32, uint32_t cluster, uint32_t partition_lba) {
  uint32_t data_start_lba =
      partition_lba + fat32->BPB_RsvdSecCnt + (fat32->BPB_NumFATs * fat32->BPB_FATSz32);
  return data_start_lba + ((cluster - 2) * fat32->BPB_SecPerClus);
}

uint32_t get_fat_entry_lba(FAT32_BPB *fat32, uint32_t cluster, uint32_t partition_lba) {
  uint32_t fat_start_lba = partition_lba + fat32->BPB_RsvdSecCnt;
  return fat_start_lba + ((cluster * 4) / fat32->BPB_BytsPerSec);
}

uint32_t get_fat_entry_offset(FAT32_BPB *fat32, uint32_t cluster) {
  return (cluster * 4) % fat32->BPB_BytsPerSec;
}

void to_dos_filename(const char *input, uint8_t *output) {
  fmemset(output, ' ', 11);
  int i = 0;

  while (input[i] != '\0' && input[i] != '.' && i < 8) {
    output[i] = toupper((unsigned char)input[i]);
    i++;
  }

  const char *dot = strchr(input, '.');
  if (dot) {
    dot++;
    int ext_len = 0;
    while (dot[ext_len] != '\0' && ext_len < 3) {
      output[8 + ext_len] = toupper((unsigned char)dot[ext_len]);
      ext_len++;
    }
  }
}

uint8_t fat32_find_file_in_dir(Disk disk, FAT32_BPB *fat32, uint32_t partition_lba, uint32_t dir_cluster,
                          const char *target_filename, uint32_t *out_start_cluster,
                          uint32_t *out_file_size) {

  uint32_t bytes_per_cluster = fat32->BPB_SecPerClus * fat32->BPB_BytsPerSec;

  uint8_t *dir_buffer = falloc(bytes_per_cluster);
  if (!dir_buffer) {
    return 0;
  }

  uint8_t dos_name[11];
  to_dos_filename(target_filename, dos_name);

  uint32_t current_dir_cluster = dir_cluster;

  while (current_dir_cluster < 0x0FFFFFF8 && current_dir_cluster > 0) {

    uint32_t dir_lba = cluster_to_lba(fat32, current_dir_cluster, partition_lba);
    lba_read(disk, dir_lba, fat32->BPB_SecPerClus, dir_buffer);

    uint32_t max_entries = bytes_per_cluster / sizeof(FAT_DirEntry);
    FAT_DirEntry *entries = (FAT_DirEntry *)dir_buffer;

    for (uint32_t i = 0; i < max_entries; i++) {
      if (entries[i].DIR_Name[0] == 0x00) {
        ffree(dir_buffer);
        return 0;
      }

      if (entries[i].DIR_Name[0] == 0xE5 || entries[i].DIR_Attr == 0x0F) {
        continue;
      }

      if (fmemcmp(entries[i].DIR_Name, dos_name, 11) == 0) {
        *out_start_cluster = ((uint32_t)entries[i].DIR_FstClusHI << 16) | entries[i].DIR_FstClusLO;
        *out_file_size = entries[i].DIR_FileSize;

        ffree(dir_buffer);
        return 1;
      }
    }

    uint32_t fat_lba = get_fat_entry_lba(fat32, current_dir_cluster, partition_lba);
    uint32_t fat_offset = get_fat_entry_offset(fat32, current_dir_cluster);

    uint8_t *fat_sector = falloc(fat32->BPB_BytsPerSec);
    if (!fat_sector) {
      ffree(dir_buffer);
      return 0;
    }

    lba_read(disk, fat_lba, 1, fat_sector);
    current_dir_cluster = (*(uint32_t *)(fat_sector + fat_offset)) & 0x0FFFFFFF;
    ffree(fat_sector);
  }

  ffree(dir_buffer);
  return 0;
}

uint8_t *fat32_read_file(Disk disk, FAT32_BPB *fat32, uint32_t partition_lba, uint32_t start_cluster,
                         uint32_t file_size) {

  uint8_t *file_buffer = falloc(file_size);
  if (!file_buffer) {
    return NULL;
  }

  uint32_t cluster_bytes = fat32->BPB_SecPerClus * fat32->BPB_BytsPerSec;
  uint8_t *cluster_buffer = falloc(cluster_bytes);
  uint8_t *fat_sector_buffer = falloc(fat32->BPB_BytsPerSec);

  if (!cluster_buffer || !fat_sector_buffer) {
    if (file_buffer) {
      ffree(file_buffer);
    }
    if (cluster_buffer) {
      ffree(cluster_buffer);
    }
    if (fat_sector_buffer) {
      ffree(fat_sector_buffer);
    }
    return NULL;
  }

  uint32_t current_cluster = start_cluster;
  uint32_t bytes_remaining = file_size;
  uint32_t buffer_offset = 0;

  while (current_cluster < 0x0FFFFFF8) {

    uint32_t cluster_lba = cluster_to_lba(fat32, current_cluster, partition_lba);
    lba_read(disk, cluster_lba, fat32->BPB_SecPerClus, cluster_buffer);

    uint32_t bytes_to_copy = (bytes_remaining < cluster_bytes) ? bytes_remaining : cluster_bytes;
    wordcpy32(file_buffer + buffer_offset, cluster_buffer, bytes_to_copy);

    bytes_remaining -= bytes_to_copy;
    buffer_offset += bytes_to_copy;

    if (bytes_remaining == 0) {
      break;
    }

    uint32_t fat_lba = get_fat_entry_lba(fat32, current_cluster, partition_lba);
    uint32_t fat_offset = get_fat_entry_offset(fat32, current_cluster);

    lba_read(disk, fat_lba, 1, fat_sector_buffer);
    current_cluster = (*(uint32_t *)(fat_sector_buffer + fat_offset)) & 0x0FFFFFFF;
  }

  ffree(cluster_buffer);
  ffree(fat_sector_buffer);

  return file_buffer;
}
