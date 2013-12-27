#export CROSS_COMPILE=arm-unknown-linux-gnueabi-
export  CROSS_COMPILE=/opt/freescale/usr/local/gcc-4.1.2-glibc-2.5-nptl-3/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-

MEM_TYPE ?= MEM_DDR1
export MEM_TYPE

DESTDIR :=../bin/

DFT_IMAGE=$(DEV_IMAGE)/boot/zImage
DFT_UBOOT=$(DEV_IMAGE)/boot/u-boot

all: gen_bootstream install

gen_bootstream: linux_prep boot_prep power_prep
	@echo "Generation linux_prep boot_prep power_prep is completed" 

power_prep:
	@echo "build power_prep"
	$(MAKE) -C power_prep

boot_prep:
	@echo "build boot_prep"
	$(MAKE) -C boot_prep

linux_prep:
	$(MAKE) clean -C linux_prep
	@echo "cross-compiling linux_prep"
	$(MAKE) -C linux_prep

install:
	@echo "Install built binaries"
	mkdir -p ${DESTDIR}
	cp -f boot_prep/boot_prep  ${DESTDIR}
	cp -f power_prep/power_prep  ${DESTDIR}
	cp -f linux_prep/output-target/linux_prep ${DESTDIR}

distclean: clean
clean:
	-rm -rf *.sb
	$(MAKE) -C linux_prep clean
	$(MAKE) -C boot_prep clean
	$(MAKE) -C power_prep clean

.PHONY: all linux_prep boot_prep power_prep distclean clean

