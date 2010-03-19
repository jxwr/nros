#pragma once
#include <ctype.h>

int check_and_clear_bit(u32 *word, int bit)
{
  /* Check that bit was set */
  if (word[BITWISE_GETWORD(bit)] & BITWISE_GETBIT(bit)) {
    word[BITWISE_GETWORD(bit)] &= ~BITWISE_GETBIT(bit);
    return 0;
  } else {
    //printf("Trying to clear already clear bit\n");
    return -1;
  }
}

int check_and_set_bit(u32 *word, int bit)
{
  /* Check that bit was clear */
  if (!(word[BITWISE_GETWORD(bit)] & BITWISE_GETBIT(bit))) {
    word[BITWISE_GETWORD(bit)] |= BITWISE_GETBIT(bit);
    return 0;
  } else {
    //printf("Trying to set already set bit\n");
    return -1;
  }
}

/* Set */
static inline void setbit(unsigned int *w, unsigned int flags)
{
  *w |= flags;
}


/* Clear */
static inline void clrbit(unsigned int *w, unsigned int flags)
{
  *w &= ~flags;
}

/* Test */
static inline int tstbit(unsigned int *w, unsigned int flags)
{
  return *w & flags;
}

/* Test and clear */
static inline int tstclr(unsigned int *w, unsigned int flags)
{
  int res = tstbit(w, flags);

  clrbit(w, flags);

  return res;
}
