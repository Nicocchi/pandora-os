#pragma once
#include "stdint.h"
#include "disk.h"

#pragma pack(push, 1)
typedef struct
{
  u8 Name[11];
  u8 Attributes;
  u8 Reserved;
  u8 CreatedTimeTenths;
  u16 CreatedTime;
  u16 CreatedDate;
  u16 AccessedDate;
  u16 FirstClusterHigh;
  u16 ModifiedTime;
  u16 ModifiedDate;
  u16 FirstClusterLow;
  u32 Size;
} FAT_DirectoryEntry;

#pragma pack(pop)

typedef struct
{
  int Handle;
  bool IsDirectory;
  u32 Position;
  u32 Size;
} FAT_File;

enum FAT_Attributes
{
  FAT_ATTRIBUTE_READ_ONLY     = 0x01,
  FAT_ATTRIBUTE_HIDDEN        = 0x02,
  FAT_ATTRIBUTE_SYSTEM        = 0x04,
  FAT_ATTRIBUTE_VOLUME_ID     = 0x08,
  FAT_ATTRIBUTE_DIRECTORY     = 0x10,
  FAT_ATTRIBUTE_ARCHIVE       = 0x20,
  FAT_ATTRIBUTE_LFN           = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN | FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID
};

bool FAT_Initialize(DISK *disk);
FAT_File far *FAT_Open(DISK *disk, const char *path);
u32 FAT_Read(DISK *disk, FAT_File far *file, u32 byteCount, void *dataOut);
bool FAT_ReadEntry(DISK *disk, FAT_File far *file, FAT_DirectoryEntry *dirEntry);
void FAT_Close(FAT_File far *file);
