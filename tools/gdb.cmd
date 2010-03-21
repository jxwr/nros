
target remote localhost:1234
# target remote 0.0.0.0:1234

# load boot/boot.elf
# symbol-file boot/boot.elf

# load setup/setup.elf
# symbol-file setup/setup.elf

# load kernel/kernel.elf
 symbol-file kernel/kernel.elf
 break main
 continue
 stepi

define mem_check
b map_vm_area
c
b 40
c
del 3
b 40 if pt_idx == 1669
c
wa *0xc0115028
c
end