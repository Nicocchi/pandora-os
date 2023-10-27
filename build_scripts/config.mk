# host
export CFLAGS = -std=c99 -g
export ASMFLAGS =
export CC = gcc
export CXX = g++
export LD = gcc
export ASM = nasm
export LDFLAGS =
export LIBS =

# os
export TARGET = i686-elf
export TARGET_ASM = nasm
export TARGET_ASMFLAGS =
export TARGET_CFLAGS = -std=c99 -g #-02
export TARGET_CC = $(abspath toolchain/$(TARGET)/bin/$(TARGET)-gcc)
export TARGET_CXX = $(abspath toolchain/$(TARGET)/bin/$(TARGET)-g++)
export TARGET_LD = $(abspath toolchain/$(TARGET)/bin/$(TARGET)-gcc)
export TARGET_LDFLAGS =
export TARGET_LIBS =

export BUILD_DIR = $(abspath build)

BINUTILS_VER = 2.41
BINUTILS_URL = https://ftp.gnu.org/gnu/binutils/binutils-$(BINUTILS_VER).tar.xz

GCC_VER = 13.2.0
GCC_URL = https://ftp.gnu.org/gnu/gcc/gcc-$(GCC_VER)/gcc-$(GCC_VER).tar.xz

