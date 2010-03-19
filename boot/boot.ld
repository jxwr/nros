OUTPUT(elf32-i386)
ENTRY(start)
SECTIONS
{
	. = 0x7c00;
	.boot : { *(.boot) }
}
