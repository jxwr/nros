OUTPUT_FORMAT(elf32-i386)
ENTRY(start)
SECTIONS
{
	. = 0x08048000;

	.text : { ../lib/crt.o(.text);
	      	  test.o(.text) }

	. = ALIGN(4);
	.data : { *(.data) }

	. = ALIGN(4);
	.bss  : { *(.bss)  }
}