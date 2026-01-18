#ifndef AMX_ASM_H
#define AMX_ASM_H

#include <stdint.h>

void amx_set(void);
void amx_clr(void);
void amx_ldx(uint64_t operand);
void amx_stx(uint64_t operand);
void amx_ldy(uint64_t operand);
void amx_sty(uint64_t operand);

#endif
