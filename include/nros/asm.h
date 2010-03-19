#pragma once
#include <nros/port.h>

static inline void cli()
{
  asm volatile("cli;");
}

static inline void sti()
{
  asm volatile("sti;");
}
