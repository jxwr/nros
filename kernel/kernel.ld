OUTPUT_FORMAT(elf32-i386)
ENTRY(start)


KVM_BASE = 0xc0000000;
KPM_BASE = 0x100000;

SECTIONS
{
	. = KVM_BASE + KPM_BASE;
	
	. = ALIGN(4);
	.head.text : AT(ADDR(.head.text) - KVM_BASE) {
		   *(.head.text)
	}

	. = ALIGN(4);
	.head.data : AT(ADDR(.head.data) - KVM_BASE) {
		   *(.head.data)
	}

	. = ALIGN(0x1000);
	.head.table : AT(ADDR(.head.table) - KVM_BASE) {
		    *(.head.table)
	}

	. = ALIGN(4);
	.text : AT(ADDR(.text) - KVM_BASE) {
	      *(.text)
	}

	. = ALIGN(4);
	.data : AT(ADDR(.data) - KVM_BASE) {
	      *(.data)
	}

	. = ALIGN(4);
	.rodata : AT(ADDR(.rodata) - KVM_BASE) {
		*(.rodata)
	}

	. = ALIGN(4);
	.bss : AT(ADDR(.bss) - KVM_BASE) {
	     *(.bss)
	}

	_img_end = .;
}