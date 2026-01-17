#include <stdio.h>

#include "amx_matmul.h"

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
    float c_amx[4][4] = {0};

    int used_amx = amx_matmul_4x4(a, b, c_amx);
    if (!used_amx) {
        printf("AMX matmul not supported on this target.\n");
        return 1;
    }
    print_mat4("Matrix A:", a);
    print_mat4("Matrix B:", b);
    print_mat4("A * B (AMX):", c_amx);

    return 0;
}
