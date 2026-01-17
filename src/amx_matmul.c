#include "amx_matmul.h"

#include <stdint.h>
#include <stdio.h>

#include "amx_asm.h"

static uint64_t amx_xy_operand(const void *ptr, uint32_t reg)
{
    return (((uint64_t)reg) << 56) | ((uint64_t)ptr & 0x00FFFFFFFFFFFFFFull);
}

static uint64_t amx_z_operand(const void *ptr, uint32_t row)
{
    return (((uint64_t)row) << 56) | ((uint64_t)ptr & 0x00FFFFFFFFFFFFFFull);
}

static uint64_t amx_fma32_operand(uint32_t x_offset, uint32_t y_offset, uint32_t z_row)
{
    uint64_t op = 0;
    op |= ((uint64_t)(x_offset & 0x1FF)) << 10;
    op |= ((uint64_t)(y_offset & 0x1FF));
    op |= ((uint64_t)(z_row & 0x3F)) << 20;
    op |= ((uint64_t)2) << 46;  /* X enable mode: first N lanes */
    op |= ((uint64_t)4) << 41;  /* X enable value: N=4 lanes */
    op |= ((uint64_t)2) << 37;  /* Y enable mode: first N lanes */
    op |= ((uint64_t)4) << 32;  /* Y enable value: N=4 lanes */
    return op;
}

int amx_matmul_4x4(const float a[4][4], const float b[4][4], float out[4][4])
{
    __attribute__((aligned(64))) float xbuf[16] = {0};
    __attribute__((aligned(64))) float ybuf[16] = {0};
    __attribute__((aligned(64))) float zbuf[16] = {0};

    amx_set();
    for (int k = 0; k < 4; k++) {
        for (int i = 0; i < 16; i++) {
            xbuf[i] = 0.0f;
            ybuf[i] = 0.0f;
        }
        for (int i = 0; i < 4; i++) {
            xbuf[i] = b[k][i];
            ybuf[i] = a[i][k];
        }

        amx_ldx(amx_xy_operand(xbuf, 0));
        // amx_ldy(amx_xy_operand(ybuf, 0));
        // amx_fma32(amx_fma32_operand(0, 0, 0));
    }

    for (int row = 0; row < 4; row++) {
        // amx_stz(amx_z_operand(zbuf, (uint32_t)(row * 4)));
        for (int col = 0; col < 4; col++) {
            out[row][col] = zbuf[col];
        }
    }
    amx_clr();
    return 1;
}

void amx_smoke(void)
{
    amx_set();
    amx_clr();
    printf("AMX set/clr issued.\n");
}
