#include <argp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define THRESHOLD 2
#define SIZE 4
#define UNIT_SIZE 8
#ifndef ROUNDS
    #define ROUNDS 32
#endif
#define ROR(x, r) ((x >> r) | (x << (16 - r)))
#define ROL(x, r) ((x << r) | (x >> (16 - r)))
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

unsigned tot_trials = 1000;
bool retry = false;

uint64_t time_elasped(struct timespec* begin, struct timespec* end) {
    int64_t output = end->tv_sec - begin->tv_sec;
    output *= 1000000000;
    output += end->tv_nsec - begin->tv_nsec;
    return output;
}

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

void ref_simon_encrypt(byte* input, byte* key, byte* output) {
    uint16_t x = input[1] | (input[0] << 8);
    uint16_t y = input[3] | (input[2] << 8);
    uint16_t keys[ROUNDS];
    uint16_t tmp;

    for (int i = 0; i < 4; ++i) {
        keys[3 - i] = key[i * 2 + 1] | (key[i * 2] << 8);
    }
    uint32_t z0 = 0b10110011100001101010010001011111;
    for (int i = 4; i < ROUNDS; ++i) {
        tmp = ROR(keys[i - 1], 3);
        tmp ^= keys[i - 3];
        tmp ^= ROR(tmp, 1);
        keys[i] = ~keys[i - 4] ^ tmp ^ 3 ^ ((z0 >> (i - 4)) & 1);
    }
    for (int i = 0; i < ROUNDS; ++i) {
        SIMON_ROUND(x, y, keys[i]);
    }
    output[0] = x >> 8;
    output[1] = x;
    output[2] = y >> 8;
    output[3] = y;
}

/* Arguments */
static char doc[] = "Test the accuracy and run time of weird machines.";
static char args_doc[] = "";
static struct argp_option options[] = {
    { "retry", 'r', 0, 0, "Retry when an error is detected"},
    { "trial", 't', "TRIAL", 0, "Number of trials"},
    { 0 } 
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    switch (key) {
        case 'r': retry = true; break;
        case 't': tot_trials = atoi(arg); break;
        case ARGP_KEY_ARG: return 0;
        default: return ARGP_ERR_UNKNOWN;
    }   
    return 0;
}
static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

int main(int argc, char* argv[]) {
    argp_parse(&argp, argc, argv, 0, 0, 0);
    srand(time(NULL));
    uint64_t tot_ns = 0;
    byte input[4], key[8], output[4], ref[4], error_output[4];
    int correct = 0, detected = 0, incorrect = 0;

    for (int i = 0; i < tot_trials; ++i) {
        char votes[4 * 8] = {0};
        bool has_error;

        ((uint32_t*)input)[0] = rand();
        ((uint32_t*)key)[0] = rand();
        ((uint32_t*)key)[1] = rand();
        ref_simon_encrypt(input, key, ref);

        // execute the circuit
        struct timespec ts_start;
        struct timespec ts_end;
        clock_gettime(CLOCK_MONOTONIC, &ts_start);
        do {
            has_error = __weird__simon_encrypt(input, key, output, error_output);
            if (update_votes(output, error_output, votes)) has_error = false;
        } while (retry && has_error);
        merge_output(output, votes);
        clock_gettime(CLOCK_MONOTONIC, &ts_end);
        tot_ns += time_elasped(&ts_start, &ts_end);

        if (has_error) {
            ++detected;
        }
        else {
            unsigned j = 0;
            for (; j < 4 && output[j] == ref[j]; ++j);
            if (j < 4) ++incorrect;
            else ++correct;
        }
    }

    printf("=== Simon32 Encryption (1 block) ===\n");
    printf("Accuracy: %.5f%%, ", (double)correct / tot_trials * 100);
    printf("Error detected: %.5f%%, ", (double)detected / tot_trials * 100);
    printf("Undetected error: %.5f%%\n", (double)incorrect / tot_trials * 100);
    uint64_t time_per_run = tot_ns / tot_trials;
    printf("Time usage: %lu.%lu (us)\n", time_per_run / 1000, time_per_run % 1000);
    printf("over %d iterations.\n", tot_trials);
    return 0;
}
