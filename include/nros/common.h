#pragma once

#include <nros/macro.h>
#include <nros/panic.h>


#define FASTCALL(x)     x __attribute__((regparm(3)))
#define fastcall          __attribute__((regparm(3)))

