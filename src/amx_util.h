#ifndef AMX_UTIL_H
#define AMX_UTIL_H

#include <stdint.h>

uint64_t amx_xy_operand(const void *ptr, uint32_t reg);
uint64_t amx_z_operand(const void *ptr, uint32_t row);

#endif
