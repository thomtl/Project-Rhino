#pragma once

#include <rhino/common.h>
#include <rhino/arch/x86/drivers/serial.h>

#ifdef DEBUG
#define debug_log(m) (debug_log_int(m))
#else
#define debug_log(m) (UNUSED(m))
#endif

#ifdef DEBUG
#define debug_log_number_dec(m) (debug_log_number_dec_int(m))
#else
#define debug_log(m) (UNUSED(m))
#endif

#ifdef DEBUG
#define debug_log_number_hex(m) (debug_log_number_hex_int(m))
#else
#define debug_log(m) (UNUSED(m))
#endif

#ifdef DEBUG

void debug_log_int(char* m);

void debug_log_number_dec_int(uint32_t num);

void debug_log_number_hex_int(uint32_t num);

#endif