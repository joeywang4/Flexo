#include <stdbool.h>
#include <stdint.h>

#define THRESHOLD 2
#define SIZE 4
#define UNIT_SIZE 8
#ifndef ROUNDS
    #define ROUNDS 32
#endif
#define ROR(x, r) ((x >> r) | (x << (16 - r)))
#define ROL(x, r) ((x << r) | (x >> (16 - r)))
#define memcpy(a, b, c) \
    for (int i = 0; i < c; ++i) (b)[i] = (a)[i];
#define SIMON_ROUND(x, y, k) \
    tmp = (ROL(x, 1) & ROL(x, 8)) ^ y ^ ROL(x, 2); \
    y = x; \
    x = tmp ^ k;
#define NEXT_KEY(k0, k1, k2, k3, z0, i) \
    tmp2 = k3; \
    tmp = ROR(k3, 3) ^ k1; \
    tmp ^= ROR(tmp, 1); \
    k3 = ~k0 ^ tmp ^ 3 ^ ((z0 >> (i - 4)) & 1); \
    k0 = k1; \
    k1 = k2; \
    k2 = tmp2;

typedef unsigned char byte;

bool update_votes(byte* output, byte* error_output, char* votes) {
    bool all_done = true;
    for (int j = 0; j < SIZE * UNIT_SIZE; ++j) {
        bool out_bit = (output[j / UNIT_SIZE] >> (j % UNIT_SIZE)) & 1;
        bool err_bit = (error_output[j / UNIT_SIZE] >> (j % UNIT_SIZE)) & 1;

        if (!err_bit) {
            if (out_bit && votes[j] < 126) ++votes[j];
            else if (!out_bit && votes[j] > -126) --votes[j];
        }
        if (votes[j] < THRESHOLD && votes[j] > -THRESHOLD) all_done = false;
    }
    return all_done;
}

void merge_output(byte* output, char* votes) {
    for (unsigned j = 0; j < SIZE * UNIT_SIZE; ++j) {
        if (votes[j] >= THRESHOLD) {
            output[j / UNIT_SIZE] |= 1 << (j % UNIT_SIZE);
        }
        else if (votes[j] <= -THRESHOLD) {
            output[j / UNIT_SIZE] &= ~(1 << (j % UNIT_SIZE));
        }
    }
}

bool __weird__simon_encrypt(byte* input, byte* key, byte* output, byte* error_output) {
    uint16_t x = input[1] | (input[0] << 8);
    uint16_t y = input[3] | (input[2] << 8);
    uint16_t k0, k1, k2, k3, k4;
    uint32_t z0 = 0b10110011100001101010010001011111;
    uint16_t tmp, tmp2;

    k0 = key[7] | (key[6] << 8);
    k1 = key[5] | (key[4] << 8);
    k2 = key[3] | (key[2] << 8);
    k3 = key[1] | (key[0] << 8);

    SIMON_ROUND(x, y, k0);
    SIMON_ROUND(x, y, k1);
    SIMON_ROUND(x, y, k2);
    SIMON_ROUND(x, y, k3);

    #pragma unroll
    for (unsigned i = 4; i < ROUNDS; ++i) {
        NEXT_KEY(k0, k1, k2, k3, z0, i);
        SIMON_ROUND(x, y, k3);
    }

    output[0] = x >> 8;
    output[1] = x;
    output[2] = y >> 8;
    output[3] = y;

    // prevent clang's optimization on error_output and the return value
    error_output[0] = input[0];
    return error_output[0];
}

void simon_encrypt(byte* input, byte* key, byte* output) {
    byte error_output[4];
    char votes[4 * 8];
    bool has_error;

    ((uint64_t*)votes)[3] = 0;
    // prevent compiler from using memset
    votes[9] = 1;
    ((uint64_t*)votes)[1] = 0;
    votes[17] = 1;
    ((uint64_t*)votes)[2] = 0;
    votes[1] = 1;
    ((uint64_t*)votes)[0] = 0;

    do {
        has_error = __weird__simon_encrypt(input, key, output, error_output);
        if (update_votes(output, error_output, votes)) has_error = false;
    } while (has_error);
    merge_output(output, votes);
}
