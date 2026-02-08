#include "fft.h"

#include <math.h>
#include <stdlib.h>

void fft2(float *x, size_t N, float *Xr, float *Xi) {
    // 0. Precompute.
    size_t levels = 0;
    size_t n = N;
    for (; n; n >>= 1)
        levels++;
    levels--;

    // 1. Bit reversal permutation.
    // NOTE Probably use a LUT for bit reversals?
    for (size_t i = 0; i < N; i++) {
        // can be done more efficiently but i wanted to get something working
        size_t ii = i, ri = 0;
        for (size_t j = 0; j < levels; j++) {
            ri |= ii & 1;
            ii >>= 1;
            ri <<= 1;
        }
        ri >>= 1;

        Xr[ri] = x[i],
        Xi[ri] = 0.0;
    }

    // 2. Iterative Butterfly stages.
    // Direct translation of the Python code.
    size_t size = 2;
    while (size <= N) {
        size_t half_size = size / 2;
        for (size_t i = 0; i < N; i += size) {
            for (size_t k = 0; k < half_size; k++) {
                // Compute twiddle factor. (Probably can precompute this as well.)
                float twiddle_r =  cosf(2 * M_PI * k / (float) size);
                float twiddle_i = -sinf(2 * M_PI * k / (float) size);

                float qr = twiddle_r * Xr[i + k + half_size] - twiddle_i * Xi[i + k + half_size];
                float qi = twiddle_r * Xi[i + k + half_size] + twiddle_i * Xr[i + k + half_size];

                Xr[i + k + half_size] = Xr[i + k] - qr;
                Xi[i + k + half_size] = Xi[i + k] - qi;

                Xr[i + k] = Xr[i + k] + qr;
                Xi[i + k] = Xi[i + k] + qi;
            }
        }
        size *= 2;
    }
}
