#include <stdio.h>
#include <stdint.h>

#include "reference/amx/aarch64.h"

static void matmul_4x4(const float a[4][4], const float b[4][4], float out[4][4])
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float sum = 0.0f;
            for (int k = 0; k < 4; k++) {
                sum += a[i][k] * b[k][j];
            }
            out[i][j] = sum;
        }
    }
}

static void print_mat4(const char *label, const float m[4][4])
{
    printf("%s\n", label);
    for (int i = 0; i < 4; i++) {
        printf("  ");
        for (int j = 0; j < 4; j++) {
            printf("%6.2f", m[i][j]);
            if (j != 3) {
                printf(" ");
            }
        }
        printf("\n");
    }
}

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
static int amx_matmul_4x4(const float a[4][4], const float b[4][4], float out[4][4])
{
    __attribute__((aligned(64))) float xbuf[16] = {0};
    __attribute__((aligned(64))) float ybuf[16] = {0};
    __attribute__((aligned(64))) float zbuf[16] = {0};

    AMX_SET();
    for (int k = 0; k < 4; k++) {
        for (int i = 0; i < 16; i++) {
            xbuf[i] = 0.0f;
            ybuf[i] = 0.0f;
        }
        for (int i = 0; i < 4; i++) {
            xbuf[i] = b[k][i];
            ybuf[i] = a[i][k];
        }

        AMX_LDX(amx_xy_operand(xbuf, 0));
        AMX_LDY(amx_xy_operand(ybuf, 0));
        AMX_FMA32(amx_fma32_operand(0, 0, 0));
    }

    for (int row = 0; row < 4; row++) {
        AMX_STZ(amx_z_operand(zbuf, (uint32_t)(row * 4)));
        for (int col = 0; col < 4; col++) {
            out[row][col] = zbuf[col];
        }
    }
    AMX_CLR();
    return 1;
}

static void amx_smoke(void)
{
    AMX_SET();
    AMX_CLR();
    printf("AMX set/clr issued.\n");
}

int main(void)
{
    printf("Hello, AMX!\n");
    amx_smoke();

    const float a[4][4] = {
        { 1.0f,  2.0f,  3.0f,  4.0f },
        { 5.0f,  6.0f,  7.0f,  8.0f },
        { 9.0f, 10.0f, 11.0f, 12.0f },
        {13.0f, 14.0f, 15.0f, 16.0f },
    };
    const float b[4][4] = {
        {16.0f, 15.0f, 14.0f, 13.0f },
        {12.0f, 11.0f, 10.0f,  9.0f },
        { 8.0f,  7.0f,  6.0f,  5.0f },
        { 4.0f,  3.0f,  2.0f,  1.0f },
    };
    float c[4][4] = {0};
    float c_amx[4][4] = {0};

    matmul_4x4(a, b, c);
    int used_amx = amx_matmul_4x4(a, b, c_amx);
    if (!used_amx) {
        printf("AMX matmul not supported on this target.\n");
    }
    print_mat4("Matrix A:", a);
    print_mat4("Matrix B:", b);
    print_mat4("A * B:", c);
    if (used_amx) {
        print_mat4("A * B (AMX):", c_amx);
    }

    return 0;
}
