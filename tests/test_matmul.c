#include <math.h>
#include <stdio.h>

#include "amx_matmul.h"

static void matmul_4x4_ref(const float a[4][4], const float b[4][4], float out[4][4])
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

static int matmul_4x4_equal(const float a[4][4], const float b[4][4], float eps)
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (fabsf(a[i][j] - b[i][j]) > eps) {
                return 0;
            }
        }
    }
    return 1;
}

int main(void)
{
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
    float ref[4][4] = {0};
    float amx[4][4] = {0};

    matmul_4x4_ref(a, b, ref);
    if (!amx_matmul_4x4(a, b, amx)) {
        printf("AMX matmul not supported on this target.\n");
        return 1;
    }

    if (!matmul_4x4_equal(ref, amx, 1e-4f)) {
        printf("AMX matmul mismatch.\n");
        return 1;
    }

    printf("AMX matmul test passed.\n");
    return 0;
}
