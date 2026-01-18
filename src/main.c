#include <stdint.h>
#include <stdio.h>

#include "amx_asm.h"
#include "amx_util.h"

void print_data(float data[4][4])
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            printf("%f%s", data[i][j], (j == 3) ? "\n" : ", ");
        }
    }
}

int main(void)
{
    float data[4][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };
    float data2[4][4] = {0};
    uint64_t operand = amx_xy_operand(data, 0);
    uint64_t operand2 = amx_xy_operand(data2, 0);

    print_data(data2);

    printf("AMX LDX demo (x0 operand).\n");
    amx_set();
    amx_ldx(operand);
    amx_stx(operand2);
    amx_clr();
    printf("AMX LDX issued.\n");

    print_data(data2);

    return 0;
}
