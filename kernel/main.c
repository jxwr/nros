#include <stdio.h>
#include <nros/bootinfo.h>
#include <nros/common.h>
#include <nros/mm.h>
#include <nros/intr.h>
#include <nros/asm.h>
#include <nros/disk_buffer.h>
#include <nros/floppy.h>
#include <nros/fat12.h>
#include <nros/proc.h>
#include <nros/tty.h>


boot_info_t* get_boot_info()
{
  return (boot_info_t*)0x9000;
}

void test_vm();

int main()
{
  boot_info_t* bootinfo = get_boot_info();

  mm_init(bootinfo->mem_size);

  intr_init();
  except_init();
  disk_buffer_init();
  fat12_init();
  proc_init();
  con_init();

  switch_to_user();
  for(;;);
}
