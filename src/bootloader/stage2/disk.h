#ifndef _DISK_H_
#define _DISK_H_

#include <stdint.h>

#define PACKED __attribute__((packed))

#define ATA_STATUS_BSY 0x80 // Busy
#define ATA_STATUS_RDY 0x40 // Drive Ready
#define ATA_STATUS_DRQ 0x08 // Data Request (Ready to transfer data)
#define ATA_STATUS_ERR 0x01 // Error occurred

#define ATA_CMD_READ 0x20 // Read command

#define Disk uint8_t

typedef struct PACKED {
  uint8_t bootable;
  uint8_t first_sector_h;
  uint8_t first_sector_s;
  uint8_t first_sector_c;
  uint8_t Partation_type;
  uint8_t last_sector_h;
  uint8_t last_sector_s;
  uint8_t last_sector_c;
  uint32_t first_sector_lba;
  uint32_t total_sectors;
} Partition;

typedef struct PACKED {
  Partition partitions[4];
} PartitionTable;

static inline void outb(uint16_t port, uint8_t val) {
  __asm__ volatile("outb %b0, %w1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
  uint8_t ret;
  __asm__ volatile("inb %w1, %b0" : "=a"(ret) : "Nd"(port));
  return ret;
}

static inline void insw(uint16_t port, void *addr, uint32_t count) {
  __asm__ volatile("cld; rep insw" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}

static inline void outsw(uint16_t port, const void *addr, uint32_t count) {
  __asm__ volatile("cld; rep outsw" : "+S"(addr), "+c"(count) : "d"(port) : "memory");
}

void ata_wait_ready();
void ata_wait_drq();
void ata_read_sector(Disk disk, uint32_t lba, uint8_t *buffer);

void lba_read(Disk disk, uint32_t lba, uint8_t sectors, uint8_t *buffer);

#endif // _DISK_H_
