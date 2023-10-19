#include "memory.h"

void far* memcpy(void far* dst, const void far* src, u16 num)
{
    u8 far* u8Dst = (u8 far *)dst;
    const u8 far* u8Src = (const u8 far *)src;

    for (u16 i = 0; i < num; i++)
        u8Dst[i] = u8Src[i];

    return dst;
}

void far * memset(void far * ptr, int value, u16 num)
{
    u8 far* u8Ptr = (u8 far *)ptr;

    for (u16 i = 0; i < num; i++)
        u8Ptr[i] = (u8)value;

    return ptr;
}

int memcmp(const void far* ptr1, const void far* ptr2, u16 num)
{
    const u8 far* u8Ptr1 = (const u8 far *)ptr1;
    const u8 far* u8Ptr2 = (const u8 far *)ptr2;

    for (u16 i = 0; i < num; i++)
        if (u8Ptr1[i] != u8Ptr2[i])
            return 1;

    return 0;
}
