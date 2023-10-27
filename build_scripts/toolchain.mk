
TOOLCHAIN_PREFIX = $(abspath toolchain/$(TARGET))
export PATH := "$(TOOLCHAIN_PREFIX)/bin:$(PATH)"

BINUTILS_SRC = toolchain/binutils-$(BINUTILS_VER)
BINUTILS_BUILD = toolchain/binutils-build-$(BINUTILS_VER)
GCC_SRC = toolchain/gcc-$(GC_VER)
GCC_BUILD = toolchain/gcc-build-$(GCC_VER)

toolchain: toolchain_binutils toolchain_gcc

toolchain_binutils: $(BINUTILS_SRC).tar.xz
	cd toolchain && tar -xf binutils-$(BINUTILS_VER).tar.xz
	mkdir $(BINUTILS_BUILD)
	cd $(BINUTILS_BUILD) && ../binutils-$(BINUTILS_VER)/configure \
		--prefix="$(TOOLCHAIN_PREFIX)" \
		--target=$(TARGET)						\
		--with-sysroot								\
		--disable-nls									\
		--disable-werror
	$(MAKE) -j8 -C $(BINUTILS_BUILD)
	$(MAKE) -C $(BINUTILS_BUILD) install

$(BINUTILS_SRC).tar.xz:
	mkdir -p toolchain
	cd toolchain && wget $(BINUTILS_URL)
	echo "--> Finished downloading Binutils"

toolchain_gcc: toolchain_binutils $(GCC_SRC).tar.xz
	cd toolchain && tar -xf gcc-$(GCC_VER).tar.xz
	mkdir $(GCC_BUILD)
	cd $(GCC_BUILD) && ../gcc-$(GCC_VER)/configure \
		--prefix="$(TOOLCHAIN_PREFIX)" \
		--target=$(TARGET)						\
		--disable-nls									\
		--enable-languages=c,c++			\
		--without-headers
	$(MAKE) -j8 -C $(GCC_BUILD) all-gcc all-target-libgcc
	$(MAKE) -C $(GCC_BUILD) install-gcc install-target-libgcc

$(GCC_SRC).tar.xz:
	mkdir -p toolchain
	cd toolchain && wget $(GCC_URL)

# Clean
clean-toolchain:
	rm -rf $(GCC_BUILD) $(GCC_SRC) $(BINUTILS_BUILD) $(BINUTILS_SRC)

clean-toolchain-all:
	rm -rf toolchain/*

.PHONY: toolchain toolchain_binutils toolchain_gcc clean-toolchain clean-toolchain-all
