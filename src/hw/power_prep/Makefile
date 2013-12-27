CURRENTDIR	= .
SRCDIR		= $(CURRENTDIR)
OUTPUTDIR	= $(CURRENTDIR)/output-target
COMMONSRCDIR	= $(CURRENTDIR)/../mach-stmp378x

INCLUDEDIRS	= -I$(COMMONSRCDIR) -I$(COMMONSRCDIR)/includes

# Linker script
BASE_ADDR	?= 0x00000000
BOOT_LAYOUT	= $(SRCDIR)/link.lds

# Output ELF image
POWER_PREP	= $(OUTPUTDIR)/power_prep

CROSS_COMPILE ?= arm-linux-gnueabi-

AS	= $(CROSS_COMPILE)as
CC	= $(CROSS_COMPILE)gcc
LD	= $(CROSS_COMPILE)ld
CPP	= $(CROSS_COMPILE)cpp
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY	= $(CROSS_COMPILE)objcopy
OBJDUMP	= $(CROSS_COMPILE)objdump

MEM_TYPE ?= MEM_DDR1

CFLAGS 	= -Wall $(INCLUDEDIRS) -I./ -O -DSTMP378x
LDFLAGS = -static -gc-sections -nostdlib -T $(BOOT_LAYOUT)


#TOP_LEVEL_OBJS = power_prep.o

#POWER_PREP_OBJS += $(COMMONSRCDIR)/hw/libhw.a
#POWER_PREP_OBJS += $(COMMONSRCDIR)/drivers/libddi.a
hw_libs += $(COMMONSRCDIR)/hw/power/libhw_power.a
hw_libs += $(COMMONSRCDIR)/hw/lradc/libhw_lradc.a
hw_libs += $(COMMONSRCDIR)/hw/icoll/libhw_icoll.a
hw_libs += $(COMMONSRCDIR)/hw/digctl/libhw_digctl.a
hw_libs += $(COMMONSRCDIR)/hw/core/libhw_core.a
ddi_libs += $(COMMONSRCDIR)/drivers/power/libddi_power.a


POWER_PREP_OBJS = power_prep.o eabi.o

# Default goal
.PHONY: all
all: power_prep

%.o: %.S
	$(CC) -c $(CFLAGS) -D__ASSEMBLY__ -o $@ $<

%.o: %.arm
	$(CC) -c $(CFLAGS) -D__ASSEMBLY__ -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<



#
# Make targets
#
.PHONY: build build_prep clean
power_prep: libs $(BOOT_LAYOUT) $(POWER_PREP_OBJS)
	$(LD) -o $@ $(POWER_PREP_OBJS) $(ddi_libs) $(hw_libs) \
	$(LDFLAGS)
	@nm -n $@ > power_prep.map

libs:
	$(MAKE) -C $(COMMONSRCDIR)/hw
	$(MAKE) -C $(COMMONSRCDIR)/drivers

clean:
	@echo Cleaning...
	$(MAKE) clean -C $(COMMONSRCDIR)/hw
	$(MAKE) clean -C $(COMMONSRCDIR)/drivers
	@echo Files:
	rm -f $(POWER_PREP_OBJS)
	@echo Build output:
	rm -f power_prep.map
	rm -f power_prep

