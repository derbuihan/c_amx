#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "amx_asm.h"
#include "amx_util.h"

static int amx_test_ldst_xy(void)
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
        return 0;
    }

    memset(dst, 0, sizeof(dst));

    amx_set();
    amx_ldy(amx_xy_operand(src, 0));
    amx_sty(amx_xy_operand(dst, 0));
    amx_clr();

    if (memcmp(src, dst, sizeof(src)) != 0) {
        printf("AMX ldy/sty mismatch.\n");
        return 0;
    }

    printf("AMX ldx/stx/ldy/sty test passed.\n");
    return 1;
}

int main(void)
{
    if (!amx_test_ldst_xy()) {
        return 1;
    }

    return 0;
}
