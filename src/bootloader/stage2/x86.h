#pragma once
#include "stdint.h"

void _cdecl x86_div64_32(u64 dividend, u32 divisor, u64 *quotientOut, u32 *remainderOut);
void _cdecl x86_Video_WriteCharTeletype(char c, u8 page);
