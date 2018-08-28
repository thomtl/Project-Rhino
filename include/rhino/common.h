#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <rhino/panic.h>
#define KERNEL_VBASE 0xC0000000

#define BIT_IS_SET(var, pos) ((var) & (1 << (pos)))
#define BIT_SET(var, pos) ((var) |= (1ULL << (pos)))
#define BIT_CLEAR(var, pos) ((var) &= ~(1ULL << (pos)))
#define ASSERT(b) ((b) ? (void)0 : panic_assert(__FILE__, __LINE__, #b))
#define CLI() __asm__ __volatile__ ("cli")
#define STI() __asm__ __volatile__ ("sti")
#define UNUSED(x) (void)(x)
#define PANIC_M(x) (panic_m(x, __FILE__))