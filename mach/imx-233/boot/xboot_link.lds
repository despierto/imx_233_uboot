OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

C_STACK_SIZE = 512;
IRQ_STACK_SIZE = 32;
FIQ_STACK_SIZE = 32;
SVC_STACK_SIZE = 32;
ABT_STACK_SIZE = 32;
UND_STACK_SIZE = 32;

MEMORY {
	ROM   (rx)  : ORIGIN = 0xFFFF0000, LENGTH = 64k
	SRAM  (rwx) : ORIGIN = 0x00000000, LENGTH = 32k
	SDRAM (rwx) : ORIGIN = 0x40000000, LENGTH = 64M
}

SECTIONS {
	.reset 0x100 : {
		. = ALIGN(4);
		*(ivt)
	} > SRAM

	.text 0x200 : {
		. = ALIGN(4);
		*(.text*)
	} > SRAM

	.data : {
		*(.data)
	} > SRAM

	.bss : {
		__bss_start__ = . ;
		*(.bss)
		__bss_end__ = . ;
	} > SRAM

	.stack 0x7a00 : {
		__stack_start__ = . ;

		. += IRQ_STACK_SIZE;
		. = ALIGN (4);
		__irq_stack_top__ = . ;

		. += FIQ_STACK_SIZE;
		. = ALIGN (4);
		__fiq_stack_top__ = . ;

		. += SVC_STACK_SIZE;
		. = ALIGN (4);
		__svc_stack_top__ = . ;

		. += ABT_STACK_SIZE;
		. = ALIGN (4);
		__abt_stack_top__ = . ;

		. += UND_STACK_SIZE;
		. = ALIGN (4);
		__und_stack_top__ = . ;

		. += C_STACK_SIZE;
		. = ALIGN (4);
		__c_stack_top__ = . ;

		__stack_end__ = .;
	} > SRAM
}
