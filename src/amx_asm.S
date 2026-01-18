.text
.align 2

.global _amx_set
_amx_set:
    nop
    nop
    nop
    .word (0x201000 + (17 << 5) + 0)
    ret

.global _amx_clr
_amx_clr:
    nop
    nop
    nop
    .word (0x201000 + (17 << 5) + 1)
    ret

.global _amx_ldx
_amx_ldx:
    .word (0x201000 + (0 << 5) + 0)
    ret

.global _amx_ldy
_amx_ldy:
    .word (0x201000 + (1 << 5) + 0)
    ret

.global _amx_stx
_amx_stx:
    .word (0x201000 + (2 << 5) + 0)
    ret

.global _amx_sty
_amx_sty:
    .word (0x201000 + (3 << 5) + 0)
    ret

.global _amx_ldz
_amx_ldz:
    .word (0x201000 + (4 << 5) + 0)
    ret

.global _amx_stz
_amx_stz:
    .word (0x201000 + (5 << 5) + 0)
    ret
