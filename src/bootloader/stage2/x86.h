#pragma once
#include "stdint.h"

void _cdecl x86_div64_32(u64 dividend, u32 divisor, u64 *quotientOut, u32 *remainderOut);

void _cdecl x86_Video_WriteCharTeletype(char c, u8 page);

bool _cdecl x86_Disk_Reset(u8 drive);
bool _cdecl x86_Disk_Read(u8 drive, u16 cylinder, u16 sector, u16 head, u8 count, void far *dataOut);

// Get disk geometry without relying on the boot sector, which could get corrupted.
bool _cdecl x86_Disk_GetDriveParams(u8 drive, u8 *driveTypeOut, u16 *cylindersOut, u16 *sectorsOut, u16 *headsOut);
