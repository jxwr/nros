OUTPUT_FORMAT(elf32-i386)
ENTRY(main)
SECTIONS
{
	. = 0x08048000;

	.text : { *(.text) }

	. = ALIGN(4);
	.data : { *(.data) }

	. = ALIGN(4);
	.bss  : { *(.bss)  }
}