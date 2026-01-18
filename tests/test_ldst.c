#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "amx_asm.h"
#include "amx_util.h"

static int amx_test_set_clr(void)
{
    amx_set();
    amx_clr();
    printf("AMX set/clr test passed.\n");
    return 0;
}

static int amx_test_ldx_stx(void)
{
    __attribute__((aligned(64))) uint8_t src[64];
    __attribute__((aligned(64))) uint8_t dst[64];

    for (uint32_t i = 0; i < 64; i++) {
        src[i] = (uint8_t)(i ^ 0xA5);
        dst[i] = 0;
    }

    amx_set();
    amx_ldx(amx_xy_operand(src, 0));
    amx_stx(amx_xy_operand(dst, 0));
    amx_clr();

    if (memcmp(src, dst, sizeof(src)) != 0) {
        printf("AMX ldx/stx mismatch.\n");
        return 1;
    }

    printf("AMX ldx/stx test passed.\n");
    return 0;
}

static int amx_test_ldy_sty(void)
{
    __attribute__((aligned(64))) uint8_t src[64];
    __attribute__((aligned(64))) uint8_t dst[64];

    for (uint32_t i = 0; i < 64; i++) {
        src[i] = (uint8_t)(i ^ 0x5A);
        dst[i] = 0;
    }

    amx_set();
    amx_ldy(amx_xy_operand(src, 0));
    amx_sty(amx_xy_operand(dst, 0));
    amx_clr();
    if (memcmp(src, dst, sizeof(src)) != 0) {
        printf("AMX ldy/sty mismatch.\n");
        return 1;
    }

    printf("AMX ldy/sty test passed.\n");
    return 0;
}

static int amx_test_ldz_stz(void)
{
    __attribute__((aligned(64))) uint8_t src[64];
    __attribute__((aligned(64))) uint8_t dst[64];

    for (uint32_t i = 0; i < 64; i++) {
        src[i] = (uint8_t)(i ^ 0x3C);
        dst[i] = 0;
    }

    amx_set();
    amx_ldz(amx_z_operand(src, 0));
    amx_stz(amx_z_operand(dst, 0));
    amx_clr();

    if (memcmp(src, dst, sizeof(src)) != 0) {
        printf("AMX ldz/stz mismatch.\n");
        return 1;
    }

    printf("AMX ldz/stz test passed.\n");
    return 0;
}

int main(void)
{
    if (amx_test_set_clr() != 0) {
        return 1;
    }

    if (amx_test_ldx_stx() != 0) {
        return 1;
    }

    if (amx_test_ldy_sty() != 0) {
        return 1;
    }

    if (amx_test_ldz_stz() != 0) {
        return 1;
    }

    return 0;
}
