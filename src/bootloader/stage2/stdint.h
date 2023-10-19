#pragma once

typedef signed char s8;
typedef signed short s15;
typedef signed long int s32;
typedef signed long long int s64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long int u32;
typedef unsigned long long int u64;

typedef u8 bool;

#define false       0
#define true        1

#define NULL        ((void*)0)
#define min(a,b)    ((a) < (b) ? (a) : (b))
#define max(a,b)    ((a) > (b) ? (a) : (b))
