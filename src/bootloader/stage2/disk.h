#pragma once

#include "stdint.h"

typedef struct {
  u8 id;
  u16 cylinder;
  u16 sectors;
  u16 heads;
} DISK;

bool DISK_Initialize(DISK *disk, u8 driveNumber);
bool DISK_ReadSectors(DISK *disk, u32 lba, u8 sectors, void far* dataOut);
