#include "amx_util.h"

uint64_t amx_xy_operand(const void *ptr, uint32_t reg)
{
    return (((uint64_t)reg) << 56) | ((uint64_t)ptr & 0x00FFFFFFFFFFFFFFull);
}
