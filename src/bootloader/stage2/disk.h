#ifndef _DISK_H_
#define _DISK_H_

#include <int.h>

#define ATA_STATUS_BSY 0x80 // Busy
#define ATA_STATUS_RDY 0x40 // Drive Ready
#define ATA_STATUS_DRQ 0x08 // Data Request (Ready to transfer data)
#define ATA_STATUS_ERR 0x01 // Error occurred

#define ATA_CMD_READ 0x20 // Read command

#define Disk u8

typedef struct PACKED {
  u8 bootable;
  u8 first_sector_h;
  u8 first_sector_s;
  u8 first_sector_c;
  u8 Partation_type;
  u8 last_sector_h;
  u8 last_sector_s;
  u8 last_sector_c;
  u32 first_sector_lba;
  u32 total_sectors;
} Partition;

typedef struct PACKED {
  Partition partitions[4];
} PartitionTable;

static inline void outb(u16 port, u8 val) {
  __asm__ volatile("outb %b0, %w1" : : "a"(val), "Nd"(port));
}

static inline u8 inb(u16 port) {
  u8 ret;
  __asm__ volatile("inb %w1, %b0" : "=a"(ret) : "Nd"(port));
  return ret;
}

static inline void insw(u16 port, void *addr, u32 count) {
  __asm__ volatile("cld; rep insw" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}

static inline void outsw(u16 port, const void *addr, u32 count) {
  __asm__ volatile("cld; rep outsw" : "+S"(addr), "+c"(count) : "d"(port) : "memory");
}

void ata_wait_ready();
void ata_wait_drq();
void ata_read_sector(Disk disk, u32 lba, u8 *buffer);

void lba_read(Disk disk, u32 lba, u8 sectors, u8 *buffer);

#endif // _DISK_H_
