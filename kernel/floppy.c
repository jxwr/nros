/*
 * floppy driver 
 * I ignored all delays, so it can only work on emulators,
 * need rewrite this someday.
 */

#include <nros/port.h>
#include <nros/common.h>
#include <nros/asm.h>
#include <nros/floppy.h>
#include <stdio.h>

#define FDC_DOR           0x3f2
#define FDC_MSR           0x3f4
#define FDC_FIFO          0x3f5
#define FDC_CCR           0x3f7

#define DOR_DRIVER0       0x00
#define DOR_RESET         0x04
#define DOR_DMA           0x08
#define DOR_DRIVER0_MOTOR 0x10

#define MSR_BUSY          0x10
#define MSR_DMA           0x20
#define MSR_DATAREG_READY 0x80

#define FDC_CMD_READ      0xe6
#define FDC_CMD_WRITE     0xc5
#define FDC_CMD_SENSEI    0x08
#define FDC_CMD_SEEK      0x0f
#define FDC_CMD_CALIBRATE 0x07

#define DMA_READ          0x46
#define DMA_WRITE         0x4a
#define DMA_MASK_REG      0x0a
#define DMA_MODE_REG      0x0b
#define DMA_FLIP_FLOP     0x0c
#define DMA_CHAN2_ADDR    0x04
#define DMA_CHAN2_COUNT   0x05

char dma_buffer[FD_BLOCK_SIZE];
unsigned long fd_irq_done = 0;

static void dma_setup()
{
  u8 addr;

  outb(DOR_DMA|DOR_DRIVER0_MOTOR|DOR_RESET, FDC_DOR);

  /* enable channel 2 */
  outb(0x06, DMA_MASK_REG);

  /* set dma buffer address */
  addr = pa(dma_buffer) & 0xff;
  outb(addr, DMA_CHAN2_ADDR);
  addr = (pa(dma_buffer) >> 8) & 0xff;
  outb(addr, DMA_CHAN2_ADDR);
  /* external page register */
  addr = (pa(dma_buffer) >> 16) & 0xff;
  outb(addr, 0x81);

  /* set dma buffer length 1024(0x400) */
  outb(0xff, DMA_CHAN2_COUNT);
  outb(0x3, DMA_CHAN2_COUNT);

  /* disable channel 2 */
  outb(0x02, DMA_MASK_REG);
}

static void dma_read_mode()
{
  outb(0x06, DMA_MASK_REG);
  outb(DMA_READ, DMA_MODE_REG);
  outb(0x02, DMA_MASK_REG);
}

static void dma_write_mode()
{
  outb(0x06, DMA_MASK_REG);
  outb(DMA_WRITE, DMA_MODE_REG);
  outb(0x02, DMA_MASK_REG);
}

/* 
 * because emulator does not have any delay, 
 * this function is not useful 
*/
static void wait_for_irq_done()
{
  while(fd_irq_done == 0)
    printf("waiting ");
  fd_irq_done = 0;
}

/*
 * read or write block with size 1024 to or from dma_buffer
 */
void fd_rw(int cmd, unsigned long sector_linear)
{
  int i;
  u8 sector, head, track;

  sector = sector_linear % 18 + 1;
  sector_linear /= 18;
  head = sector_linear % 2;
  track = sector_linear / 2;

  dma_setup();

  if(cmd == DISK_READ) {
    dma_read_mode();
    outb(FDC_CMD_READ, FDC_FIFO);
  }
  else if(cmd == DISK_WRITE){
    dma_write_mode();
    outb(FDC_CMD_WRITE, FDC_FIFO);
  }
  else
    BUG_MSG("fd_rw: unknown cmd.");

  /* head current_drive*/
  outb(head<<2, FDC_FIFO);
  /* track */
  outb(track, FDC_FIFO);
  /* head */
  outb(head, FDC_FIFO);
  /* sector */
  outb(sector, FDC_FIFO);
  /* sector size */
  outb(2, FDC_FIFO);
  /* track length */
  outb(18, FDC_FIFO);
  /* gap length */
  outb(0x1b, FDC_FIFO);
  /* data size */
  outb(0xff, FDC_FIFO);
  wait_for_irq_done();

  for(i = 0; i < 7; i++) {
    inb(FDC_FIFO);
  }

  /* interrupt finish */
  outb(0x08,FDC_FIFO);
  inb(FDC_FIFO);
  inb(FDC_FIFO);

  /* stop floppy motor */
  outb(0, FDC_DOR);
}

void fd_init()
{
  fd_rw(DISK_READ, 10);
  
  fd_rw(DISK_WRITE, 0);
}
