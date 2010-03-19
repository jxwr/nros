	.code16
	
	.section .boot, "ax"
	.global start	
start:
	jmp start0
	nop

oem_name:	.ascii	"12345678"
bytes_per_sec:	.short	0
sec_per_clust:	.byte	0
rsvd_sec_cnt:	.short	0
num_fats:	.byte	0
root_ent_cnt:	.short	0
tot_sect:	.short	0
media:		.byte	0
sec_per_fat:	.short	0
sec_per_trk:	.short	0
num_heads:	.short	0
hidd_sec_l:	.short	0
hidd_sec_h:	.short	0
tot_sec32:	.long	0
drv_num:	.byte	0
reserved1:	.byte	0
boot_sig:	.byte	0
vol_id:		.long	0
vol_lab:	.ascii	"12345678901"
filesys_type:	.ascii	"12345678"

start0:	
	cli
	cld
	xorw %ax,%ax
	movw %ax,%ds
	movw %ax,%es
	movw %ax,%ss
	movw $0x7c00,%sp

	/*
	 * set video mode 80X25
	 */
	movw $0x03,%ax
	int $0x10

	movw $bootmsg,%si
	call putstr
	
	/*
	 * fat start = rsvd_sec_cnt + hidd_sec, for fat12 hidd_sec is zero
	 * fat_start = 1
	 */
	movw hidd_sec_l,%si
	movw hidd_sec_h,%di
	addw rsvd_sec_cnt,%si
	movw %si,fat_start_l
	movw %di,fat_start_h

	/*
	 * root start = fat_start + sec_per_fat * numfats
	 * root_start = 19 = 1 + 2 * 9
	 */
	xorw %ax,%ax
	movb num_fats,%al
	mulw sec_per_fat
	addw %ax,%si
	adcw %dx,%di
	movw %si,root_start_l
	movw %di,root_start_h
	
	/*
	 * data start = root_start + root_ent_cnt * 32 / bytes_per_sec
	 * data_start = 33 = 19 + 224 * 32 / 512
	 */
	movw root_ent_cnt,%ax
	shlw $5,%ax
	xorw %dx,%dx
	divw bytes_per_sec
	movw %ax,root_sec_cnt
	addw %ax,%si
	adcw $0,%di
	movw %si,data_start_l
	movw %di,data_start_h

	
.set root_location,0x1000
.set fat_location,0x3000
	
	# load root at 0x1000, about 7k
	xorw %dx,%dx
	movw %dx,%es
	movw $root_location,%bx
	movw root_start_l,%ax
	movw root_start_h,%dx
	movw root_sec_cnt,%cx
	call read_sectors
	
	# load fat at 0x3000, about 5k
	xorw %dx,%dx
	movw %dx,%es
	movw $fat_location,%bx
	movw fat_start_l,%ax
	movw fat_start_h,%dx
	movw sec_per_fat,%cx
	call read_sectors
	
	# load setup code at 0x8000
	movw $filename,%si
	movw $0x8000,%bx
	call load_file

	pushw $putstr
	pushw $load_file	# save load routine in the stack
dbg0:	
	jmpw $0,$0x8000		# goto setup
	
/*
 * IN:  ds:si = filename, es:bx = buffer
 * OUT: si = filesize_l, di = filesize_h
 */
load_file:	pusha
		movw root_ent_cnt,%dx
		movw $root_location,%di
		pushw %es		# for es:di
		xorw %ax,%ax
		movw %ax,%es
		;;; 
load_file0:	movw $11,%cx		# search file
		pushw %si
		pushw %di
		repe cmpsb
		popw %di
		popw %si
		je load_file1		# found then load
		decw %dx	
		jz load_failed		# file not found
		addw $32,%di		# check next entry
		jmp load_file0

load_file1:	popw %es
		movw 0x1a(%di),%ax	# get logical cluster number
		movw 0x1c(%di),%si
		movw %si,filesize_l
		movw 0x1e(%di),%si
		movw %si,filesize_h
		cmp $0xff8,%ax
		jnb load_file3		# end
		;;; 
load_file2:	movw %ax,%si
		decw %ax		# real sector = 
		decw %ax		#  logical sector(ax) + 
		addw data_start_l,%ax	#  data_start(33) - 2
		xorw %dx,%dx
		adcw data_start_h,%dx
		call read_sector	# one sector each time
		addw bytes_per_sec,%bx	# update buffer pointer
		;;;
		movw %si,%ax		# must be ax * 3 / 2,
		addw %si,%si		# not (ax / 2) * 3
		addw %ax,%si
		shrw $1,%si
		movw fat_location(%si),%ax
		jnc load_file4
		shrw $4,%ax
		;;; 
load_file4:	andb $0x0f,%ah
		cmpw $0xff8,%ax
		jnb load_file3
		jmp load_file2
		;;; 
load_file3:	popa
		movw filesize_l,%si
		movw filesize_h,%di
		ret
	
load_failed:	movw $loadfailedmsg,%si
		call putstr
		jmp die
	
/*
 * dx:ax = start sector, es:bx = buffer
 */
read_sector:	pusha
		divw sec_per_trk
		inc %dl
		movb %dl,%cl		# cl = sector number
		xorw %dx,%dx
		divw num_heads
		movb %al,%ch		# ch = track number
		movb %dl,%dh		# dh = disk head number
		xorb %dl,%dl		# drive number, floppy 0
		movw $0x0201,%ax
		int $0x13
		popa
		ret

/*
 * dx:ax = start sector, es:bx = buffer, cx = count
 */
read_sectors:	pusha
read_sectors0:	call read_sector
		addw bytes_per_sec,%bx
		jnc read_sectors1
		pushw %dx
		movw %es,%dx
		addw $0x1000,%dx
		movw %dx,%es
		popw %dx
		;;; 
read_sectors1:	incw %ax
		adcw $0,%dx
		loop read_sectors0
		popa
		ret

die:	hlt
	jmp die

/*
 * si = address of str to display
 */
putstr:
		pusha
putstr0:	movb (%si),%al
		cmpb $0,%al
		jz putstr1
		call putchar
		incw %si
		jmp putstr0
putstr1:	popa
		ret

/*
 * al = char to display
 */
putchar:	pusha
		movb $0x0e,%ah
		xorw %bx,%bx
		int $0x10
		popa
		ret

#################
	
bootmsg:	.asciz	"booting...\n\r"
loadfailedmsg:	.asciz	"load failed...\n\r"
filename:	.asciz	"SETUP      "
filesize_l:	.short 	0
filesize_h:	.short 	0	

fat_start_l:	.short	0
fat_start_h:	.short	0
root_start_l:	.short	0
root_start_h:	.short	0
root_sec_cnt:	.short 	0
data_start_l:	.short	0
data_start_h:	.short	0
	
	.org	510,0
	.short	0xaa55