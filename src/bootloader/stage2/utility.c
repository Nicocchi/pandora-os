#include "utility.h"

u32 align(u32 number, u32 alignTo)
{
  if (alignTo == 0) {
    return number;
  }

  u32 rem = number % alignTo;
  return (rem > 0) ? (number + alignTo - rem) : number;
}
