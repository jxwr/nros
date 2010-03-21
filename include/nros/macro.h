#pragma once

#define KVM_BASE       0xC0000000
#define KVM_STACK      KVM_BASE+0x10000
#define KCODE_SEL      0x08
#define KDATA_SEL      0x10
#define UCODE_SEL      0x1B
#define UDATA_SEL      0x23
#define TSS_SEL        0x28

/*
 * defines below are from OrzMicrokernel by Jserv
 */ 
#define	ACC_GRANULARY	0b1000000000000000 
#define	ACC_DEF32SIZE	0b0100000000000000 
#define	ACC_AVL		0b0001000000000000 
#define ACC_LIMIT_MASK	0b0000111100000000 
#define ACC_LIMIT_MUL	0b0000000100000000 
#define ACC_PRESENT	0b10000000 
#define ACC_DPL_RING3	0b01100000 
#define ACC_DPL_RING2	0b01000000 
#define ACC_DPL_RING1	0b00100000 
#define ACC_DPL_RING0	0b00000000 
#define	ACC_USERSEG	0b00010000 
#define ACC_SYSTEMSEG	0b00000000 
#define ACC_TYPE_CODE	0b00001000 
#define ACC_TYPE_DATA	0b00000000 
#define ACC_TYPE_EXPDN	0b00000100 	// TYPE_DATA
#define ACC_TYPE_WRITE	0b00000010 	// TYPE_DATA
#define ACC_TYPE_CONF	0b00000100 	// TYPE_CODE
#define ACC_TYPE_READ	0b00000010 	// TYPE_CODE
#define ACC_ACCESSED	0b00000001 


#define	MASTER_PIC	0x20
#define SLAVE_PIC	0xA0
#define	IRQ_LO_PORT	0x21
#define	IRQ_HI_PORT	0xA1
#define END_OF_INTERR	0x20
#define	KBD_PORT_A	0x60
#define	KBD_PORT_B	0x61
#define KBD_PORT_C	0x64

#define pa(x) (((unsigned long)x)-KVM_BASE)
#define asm_pa(x) ((x)-KVM_BASE)

#define ALIGN(x,a) ((x+a-1)&~(a-1))
#define LONG_ALIGN(x) ALIGN(x,4)


#define true  1
#define false 0
/* Functions for critical path optimizations */
#define unlikely(x)		__builtin_expect((x), false)
#define likely(x)		__builtin_expect((x), true)
#define likelyval(x,val)	__builtin_expect((x), (val))

#ifndef NULL
#define NULL			0
#endif

#define container_of(linkptr, STRUCT, member) \
  ((STRUCT*)((char*)&(linkptr)->next - __builtin_offsetof(STRUCT, member.next)))



#define BUG()	{							\
    do {								\
      printf("BUG in file: %s function: %s line: %d\n",			\
	     __FILE__, __FUNCTION__, __LINE__);				\
    } while(0);								\
    while(1);								\
  }

#define BUG_ON(x) { \
    if (x) BUG();   \
  }

#define WARN_ON(x) {						\
    if (x)							\
      printf("%s, %s, %s: Warning something is off here.\n",	\
	     __FILE__, __FUNCTION__, __LINE__);			\
  }

#define BUG_ON_MSG(msg, x)					\
  do {								\
    printf(msg);						\
    BUG_ON(x);							\
  } while(0)

#define BUG_MSG(msg...)						\
  do {								\
    printf(msg);						\
    BUG();							\
  } while(0)
