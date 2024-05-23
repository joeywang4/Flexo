#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "aes.h"

#define ITER 100000
// #define DEBUG

unsigned tot_trials = ITER;
bool retry = false;

bool __weird__sbox(byte* in, byte* output)
{
    uint8_t input = in[0];
    SBOX_IMPL(input);
    output[0] = out;
    return out;
}

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
    unsigned correct = 0, detected = 0, incorrect = 0;

    for (unsigned i = 0; i < tot_trials; ++i) {
        byte input[1], output[1] = {0};
        byte val = rand() % 256;
        input[0] = val;

        // execute the circuit
        struct timespec ts_start;
        struct timespec ts_end;
        clock_gettime(CLOCK_MONOTONIC, &ts_start);
        bool has_error = __weird__sbox(input, output);
        while (retry && (has_error = __weird__sbox(input, output)));
        clock_gettime(CLOCK_MONOTONIC, &ts_end);
        tot_ns += time_elasped(&ts_start, &ts_end);

        #ifdef DEBUG
        printf("Input: 0x%X\n", val);
        printf("Output: 0x%X\n", output[0]);
        printf("Expected: 0x%X\n", sbox_table[val]);
        #endif

        if (has_error) {
            ++detected;
        }
        else if (sbox_table[val] == output[0]) {
            ++correct;
        }
        else {
            ++incorrect;
        }
    }

    printf("=== AES S-box ===\n");
    printf("Accuracy: %.5f%%, ", (double)correct / tot_trials * 100);
    printf("Error detected: %.5f%%, ", (double)detected / tot_trials * 100);
    printf("Undetected error: %.5f%%\n", (double)incorrect / tot_trials * 100);
    uint64_t time_per_run = tot_ns / tot_trials;
    printf("Time usage: %lu.%lu (us)\n", time_per_run / 1000, time_per_run % 1000);
    printf("over %d iterations.\n", tot_trials);
    return 0;
}
