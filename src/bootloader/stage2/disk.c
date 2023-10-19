#include "disk.h"
#include "x86.h"

bool DISK_Initialize(DISK *disk, u8 driveNumber)
{
  u8 driveType;
  u16 cylinders, sectors, heads;


  if (!x86_Disk_GetDriveParams(disk->id, &driveType, &cylinders,  &sectors, &heads)) {
    return false;
  }
  
  disk->id = driveNumber;
  disk->cylinder = cylinders + 1;
  disk->sectors = sectors;
  disk->heads = heads + 1;

  return true;
}

// Convert LBA to CHS
void DISK_LBA2CHS(DISK *disk, u32 lba, u16 *cylinderOut, u16 *sectorOut, u16 *headOut)
{
  // sector = (LBA % sectors per track + 1)
  *sectorOut = lba % disk->sectors + 1;

  // cylinder = (LBA / sectors per track) / heads
  *cylinderOut = (lba / disk->sectors) / disk->heads;

  // head = (LBA / sectors per track) % heads
  *headOut = (lba / disk->sectors) % disk->heads;
}

bool DISK_ReadSectors(DISK *disk, u32 lba, u8 sectors, void far* dataOut)
{
  u16 cylinder, sector, head;
  DISK_LBA2CHS(disk, lba, &cylinder, &sector, &head);

  // Attempt to read from the disk a couple of times
  // floppy drives are unstable irl
  for (int i = 0; i < 3; i++)
  {
    if (x86_Disk_Read(disk->id, cylinder, sector, head, sectors, dataOut)) {
      return true;
    }

    x86_Disk_Reset(disk->id);
  }

  return false;
}
