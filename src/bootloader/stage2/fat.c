#include "fat.h"
#include "stdio.h"
#include "memdefs.h"
#include "utility.h"
#include "string.h"
#include "memory.h"
#include "ctype.h"

#define SECTOR_SIZE           512
#define MAX_PATH_SIZE         256
#define MAX_FILE_HANDLES      10
#define ROOT_DIRECTORY_HANDLE -1

#pragma pack(push, 1)
typedef struct
{
  u8 BootJumpInstruction[3];
  u8 OemIdentifier[8];
  u16 BytesPerSector;
  u8 SectorsPerCluster;
  u16 ReservedSectors;
  u8 FatCount;
  u16 DirEntryCount;
  u16 TotalSectors;
  u8 MediaDescriptorType;
  u16 SectorsPerFat;
  u16 SectorsPerTrack;
  u16 Heads;
  u32 HiddenSectors;
  u32 LargeSectorCount;

  // Extended boot record
  u8 DriveNumber;
  u8 _Reserved;
  u8 Signature;
  u32 VolumeId;        // serial number, value doesn't matter
  u8 VolumeLabel[11];  // 11 bytes, padded with spaces
  u8 SystemId[8];
} FAT_BootSector;

#pragma pack(pop)

typedef struct
{
  FAT_File Public;
  bool Opened;
  u32 FirstCluster;
  u32 CurrentCluster;
  u32 CurrentSectorInCluster;
  u8 Buffer[SECTOR_SIZE];

} FAT_FileData;

typedef struct
{
  union
  {
    FAT_BootSector BootSector;
    u8 BootSectorBytes[SECTOR_SIZE];
  } BS;

  FAT_FileData RootDirectory;

  FAT_FileData OpenedFiles[MAX_FILE_HANDLES];

} FAT_Data;

static FAT_Data far* g_Data;
static u8 far* g_Fat = NULL;
static u32 g_DataSectionLba;

bool FAT_ReadBootSector(DISK *disk)
{
  return DISK_ReadSectors(disk, 0, 1, g_Data->BS.BootSectorBytes);
}

// Read the File Allocation Table into memory
bool FAT_ReadFat(DISK *disk)
{
  return DISK_ReadSectors(disk, g_Data->BS.BootSector.ReservedSectors, g_Data->BS.BootSector.SectorsPerFat, g_Fat);
}

bool FAT_Initialize(DISK *disk)
{
  g_Data = (FAT_Data far*)MEMORY_FAT_ADDR;

  // read boot sector
  if (!FAT_ReadBootSector(disk)) {
    printf("FAT: read boot sector failed\r\n");
    return false;
  }

  // read fat
  g_Fat = (u8 far*)g_Data + sizeof(FAT_Data);
  u32 fatSize = g_Data->BS.BootSector.BytesPerSector * g_Data->BS.BootSector.SectorsPerFat;
  
  if (sizeof(FAT_Data) + fatSize >= MEMORY_FAT_SIZE) {
    printf("FAT: not enough memory to read FAT! Required %lu, only have %u\r\n", sizeof(FAT_Data) + fatSize, MEMORY_FAT_SIZE);
    return false;
  }

  if (!FAT_ReadFat(disk))
  {
    printf("FAT: read FAT failed\r\n");
    return false;
  }

  // open root directory
  u32 rootDirLba = g_Data->BS.BootSector.ReservedSectors + g_Data->BS.BootSector.SectorsPerFat * g_Data->BS.BootSector.FatCount;
  u32 rootDirSize = sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;

  // open root directory file
  g_Data->RootDirectory.Public.Handle = ROOT_DIRECTORY_HANDLE;
  g_Data->RootDirectory.Public.IsDirectory = true;
  g_Data->RootDirectory.Public.Position = 0;
  g_Data->RootDirectory.Public.Size = sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;
  g_Data->RootDirectory.Opened = true;
  g_Data->RootDirectory.FirstCluster = rootDirLba;
  g_Data->RootDirectory.CurrentCluster = rootDirLba;
  g_Data->RootDirectory.CurrentSectorInCluster = 0;

  if (!DISK_ReadSectors(disk, rootDirLba, 1, g_Data->RootDirectory.Buffer)) {
    printf("FAT: read root directory failed\r\n");
    return false;
  }

  // calculate data section
  u32 rootDirSectors = (rootDirSize + g_Data->BS.BootSector.BytesPerSector - 1) /g_Data->BS.BootSector.BytesPerSector;
  g_DataSectionLba = rootDirLba + rootDirSectors;

  // reset opened files
  for (int i = 0; i < MAX_FILE_HANDLES; i++) {
    g_Data->OpenedFiles[i].Opened = false;
  }

  return true;
}

u32 FAT_ClusterToLba(u32 cluster)
{
  return g_DataSectionLba + (cluster - 2) * g_Data->BS.BootSector.SectorsPerCluster;
}

FAT_File far *FAT_OpenEntry(DISK *disk, FAT_DirectoryEntry *entry)
{
  // find empty handle
  int handle = -1;
  for (int i = 0; i < MAX_FILE_HANDLES && handle < 0; i++) {
    if (!g_Data->OpenedFiles[i].Opened) {
      handle = i;
    }
  }

  // out of handles
  if (handle < 0) {
    printf("FAT: out of file handles\r\n");
    return false;
  }

  // setup vars
  FAT_FileData far *fd = &g_Data->OpenedFiles[handle];
  fd->Public.Handle = handle;
  fd->Public.IsDirectory = (entry->Attributes & FAT_ATTRIBUTE_DIRECTORY) != 0;
  fd->Public.Position = 0;
  fd->Public.Size = entry->Size;
  fd->FirstCluster = entry->FirstClusterLow + ((u32)entry->FirstClusterHigh << 16);
  fd->CurrentCluster = fd->FirstCluster;
  fd->CurrentSectorInCluster = 0;

  if (!DISK_ReadSectors(disk, FAT_ClusterToLba(fd->CurrentCluster), 1, fd->Buffer))
  {
    printf("FAT: read error\r\n");
    return false;
  }

  fd->Opened = true;
  return &fd->Public;
}

u32 FAT_NextCluster(u32 currentCluster)
{
  u32 fatIndex = currentCluster * 3 / 2;

  if (currentCluster % 2 == 0) {
    return (*(u16 far*)(g_Fat + fatIndex)) & 0x0FFF;
  } else {
    return (*(u16 far*)(g_Fat + fatIndex)) >> 4;
  }
}

u32 FAT_Read(DISK *disk, FAT_File far *file, u32 byteCount, void *dataOut)
{
  // get file data
  FAT_FileData far* fd = (file->Handle == ROOT_DIRECTORY_HANDLE) 
        ? &g_Data->RootDirectory 
        : &g_Data->OpenedFiles[file->Handle];

  u8 *u8DataOut = (u8*)dataOut;

  // don't read past the end of the file
  if (!fd->Public.IsDirectory) {
    byteCount = min(byteCount, fd->Public.Size - fd->Public.Position);
  }

  while (byteCount > 0) {
    u32 leftInBuffer = SECTOR_SIZE - (fd->Public.Position % SECTOR_SIZE);
    u32 take = min(byteCount, leftInBuffer);

    memcpy(u8DataOut, fd->Buffer + fd->Public.Position % SECTOR_SIZE, take);
    u8DataOut += take;
    fd->Public.Position += take;
    byteCount -= take;

    // see if we need to read more data
    if (leftInBuffer == take) {
      // Special handling for root directory
      if (fd->Public.Handle == ROOT_DIRECTORY_HANDLE) {
        ++fd->CurrentCluster;

        // read next sector
        if (!DISK_ReadSectors(disk, fd->CurrentCluster, 1, fd->Buffer)) {
          printf("FAT: read error!\r\n");
          break;
        }
      } else {
        // calculate next cluster & sector to read
        if (++fd->CurrentSectorInCluster >= g_Data->BS.BootSector.SectorsPerCluster) {
          fd->CurrentSectorInCluster = 0;
          fd->CurrentCluster = FAT_NextCluster(fd->CurrentCluster);
        }

        if (fd->CurrentCluster >= 0xFF8) {
          // makr end of the file
          fd->Public.Size = fd->Public.Position;
          break;
        }

        // read next sector
        if (!DISK_ReadSectors(disk, FAT_ClusterToLba(fd->CurrentCluster) + fd->CurrentSectorInCluster, 1, fd->Buffer)) {
          printf("FAT: read error!\r\n");
          break;
        }
      }
    }
  }

  return u8DataOut - (u8*)dataOut;
}

bool FAT_ReadEntry(DISK *disk, FAT_File far *file, FAT_DirectoryEntry *dirEntry)
{
  return FAT_Read(disk, file, sizeof(FAT_DirectoryEntry), dirEntry) == sizeof(FAT_DirectoryEntry);
}

void FAT_Close(FAT_File far *file)
{
  if (file->Handle == ROOT_DIRECTORY_HANDLE) {
    file->Position = 0;
    g_Data->RootDirectory.CurrentCluster = g_Data->RootDirectory.FirstCluster;
  } else {
    g_Data->OpenedFiles[file->Handle].Opened = false;
  }
}

bool FAT_FindFile(DISK *disk, FAT_File far *file, const char *name, FAT_DirectoryEntry *entryOut)
{
  char fatName[12];
  FAT_DirectoryEntry entry;

  // convert from name to fat name
  memset(fatName, ' ', sizeof(fatName));
  fatName[11] = '\0';

  const char *ext = strchr(name, '.');
  if (ext == NULL) {
    ext = name + 11;
  }

  for (int i = 0; i < 8 && name[i] && name + i < ext; i++) {
    fatName[i] = toupper(name[i]);
  }

  if (ext != NULL) {
    for (int i = 0; i < 3 && ext[i + 1]; i++) {
      fatName[i + 8] = toupper(ext[i + 1]);
    }
  }

  while (FAT_ReadEntry(disk, file, &entry)) {
    if (memcmp(fatName, entry.Name, 11) == 0) {
      *entryOut = entry;
      return true;
    }
  }

  return false;
}

FAT_File far* FAT_Open(DISK *disk, const char *path)
{
  char name[MAX_PATH_SIZE];

  // ignore leading slash
  if (path[0] == '/') {
    path++;
  }

  FAT_File far *current = &g_Data->RootDirectory.Public;

  while (*path) {
    // extract next file name from path
    bool isLast = false;
    const char *delim = strchr(path, '/');
    if (delim != NULL) {
      memcpy(name, path, delim - path);
      name[delim - path + 1]= '\0';
      path = delim + 1;
    } else {
      unsigned len = strlen(path);
      memcpy(name, path, len);
      name[len + 1] = '\0';
      path += len;
      isLast = true;
    }

    // find directory entry in current directory
    FAT_DirectoryEntry entry;
    if (FAT_FindFile(disk, current, name, &entry)) {
      FAT_Close(current);

      // check if directory
      if (!isLast && entry.Attributes & FAT_ATTRIBUTE_DIRECTORY == 0) {
        printf("FAT: %s not a directory\r\n", name);
        return NULL;
      }

      // open new directory entry
      current = FAT_OpenEntry(disk, &entry);
    } else {
      FAT_Close(current);

      printf("FAT: %s not found\r\n", name);
      return NULL;
    }
  }

  return current;
}