#pragma once
#include "stdint.h"

void far* memcpy(void far* dst, const void far* src, u16 num);
void far* memset(void far* ptr, int value, u16 num);
int memcmp(const void far* ptr1, const void far* ptr2, u16 num);
