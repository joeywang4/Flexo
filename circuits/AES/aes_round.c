#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "aes.h"

unsigned tot_trials = 1000;
bool retry = false;

inline __attribute__((always_inline)) byte sbox(byte input)
{
    SBOX_IMPL(input);
    return out;
}

bool __weird__aes_round(byte* input, byte* key, byte* output, byte* error_output) {
    // prevent clang's optimization on error_output and the return value
    error_output[0] = input[0];

    Sbox_ShiftRows(input, sbox);
    MixColumns_RoundKey(key, output);
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
    byte input[16], key[16], output[16] = {0}, error_output[16], ref[16] = {0};
    char votes[SIZE] = {0};
    bool has_error;

    int correct = 0, detected = 0, incorrect = 0;
    for (unsigned i = 0; i < tot_trials; ++i) {
        for (unsigned j = 0; j < 4; ++j) {
            ((unsigned*)input)[j] = rand();
            ((unsigned*)key)[j] = rand();
        }
        ref_aes_round(input, key, ref);

        // execute the circuit
        struct timespec ts_start;
        struct timespec ts_end;
        clock_gettime(CLOCK_MONOTONIC, &ts_start);
        do {
            has_error = __weird__aes_round(input, key, output, error_output);
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
            for (; j < 16 && output[j] == ref[j]; ++j);
            if (j == 16) ++correct;
            else ++incorrect;
        }
    }

    printf("=== AES Round ===\n");
    printf("Accuracy: %.5f%%, ", (double)correct / tot_trials * 100);
    printf("Error detected: %.5f%%, ", (double)detected / tot_trials * 100);
    printf("Undetected error: %.5f%%\n", (double)incorrect / tot_trials * 100);
    uint64_t time_per_run = tot_ns / tot_trials;
    printf("Time usage: %lu.%lu (us)\n", time_per_run / 1000, time_per_run % 1000);
    printf("over %d iterations.\n", tot_trials);
    return 0;
}
