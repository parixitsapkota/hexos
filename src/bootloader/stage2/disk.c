#include "disk.h"

void ata_wait_ready() {
  // Loop while the BSY (Busy) bit is set
  while (inb(0x1F7) & ATA_STATUS_BSY)
    ;
}

void ata_wait_drq() {
  // Loop until the DRQ (Data Request) bit is set
  while (!(inb(0x1F7) & ATA_STATUS_DRQ))
    ;
}

void ata_read_sector(Disk disk, u32 lba, u8 *buffer) {
  ata_wait_ready();

  outb(0x1F6, disk | ((lba >> 24) & 0x0F));

  // 2. Specify that we want to read exactly 1 sector
  outb(0x1F2, 1);

  // 3. Pass the remaining LBA bits down to the ports
  outb(0x1F3, (u8)lba);         // LBA bits 0-7
  outb(0x1F4, (u8)(lba >> 8));  // LBA bits 8-15
  outb(0x1F5, (u8)(lba >> 16)); // LBA bits 16-23

  // 4. Send the READ SECTORS command
  outb(0x1F7, ATA_CMD_READ);

  // 5. Wait for the hardware to process and present data
  ata_wait_ready();
  ata_wait_drq();

  // 6. Read 256 words (512 bytes) from the Data Port into our buffer
  insw(0x1F0, buffer, 256);
}

void lba_read(Disk disk, u32 lba, u8 sectors, u8 *buffer) {
  for (u32 i = 0; i < sectors; ++i) {
    u8 *current_sector_dest = buffer + (i * 512);
    ata_read_sector(disk, lba + i, current_sector_dest);
  }
}
