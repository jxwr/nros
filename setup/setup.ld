OUTPUT_FORMAT(elf32-i386)
ENTRY(start)
SECTIONS
{
	. = 0x8000;
	
	. = ALIGN(4);
	.setup.text : { *(.setup.text) }

	. = ALIGN(4);
	.setup.data : { *(.setup.data) }

	. = ALIGN(4);
	.text : { *(.text) }

	. = ALIGN(4);
	.data : { *(.data) }

	. = ALIGN(4);
	.bss : { *(.bss) }
}