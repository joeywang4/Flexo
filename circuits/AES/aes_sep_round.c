#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "aes.h"

#define ITER 100000

unsigned tot_trials = ITER;
bool retry = false;

bool __weird__sbox(byte* in, byte* output)
{
    uint8_t input = in[0];
    SBOX_IMPL(input);
    output[0] = out;
    return out;
}

bool __weird__aes_round(byte* input, byte* key, byte* output) {
    ShiftRows(input);
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
    byte input[16] = {
        0xDE, 0xAD, 0xBE, 0xEF,
        0xFA, 0xCE, 0xB0, 0x0C,
        0xDE, 0xAD, 0xBE, 0xEF,
        0xFA, 0xCE, 0xB0, 0x0C,
    };
    byte key[16] = {
        0x00, 0x11, 0x22, 0x33,
        0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xAA, 0xBB,
        0xCC, 0xDD, 0xEE, 0xFF,
    };
    byte output[16] = {0};
    byte ref[16] = {0};


    int correct = 0, detected = 0, incorrect = 0;
    for (int i = 0; i < tot_trials; ++i) {
        byte sbox_out[16] = {0};

        // execute the circuit
        struct timespec ts_start;
        struct timespec ts_end;
        clock_gettime(CLOCK_MONOTONIC, &ts_start);
        for (int i = 0; i < 16; ++i) {
            input[i] = rand() % 256;
            key[i] = rand() % 256;
            byte sbox_in[1] = {input[i]};
            bool has_error = __weird__sbox(sbox_in, sbox_out + i);
            if (has_error) --i;
        }

        bool has_error = __weird__aes_round(sbox_out, key, output);
        while (retry && (has_error = __weird__aes_round(sbox_out, key, output)));
        clock_gettime(CLOCK_MONOTONIC, &ts_end);
        tot_ns += time_elasped(&ts_start, &ts_end);

        ref_aes_round(input, key, ref);

        if (has_error) {
            ++detected;
        }
        else {
            bool is_incorrect = false;
            for (int j = 0; j < 16; ++j) {
                if (output[j] != ref[j]) {
                    is_incorrect = true;
                    break;
                }
            }
            if (is_incorrect) ++incorrect;
            else ++correct;
        }
    }

    printf("=== AES Round (separate s-box) ===\n");
    printf("Accuracy: %.5f%%, ", (double)correct / tot_trials * 100);
    printf("Error detected: %.5f%%, ", (double)detected / tot_trials * 100);
    printf("Undetected error: %.5f%%\n", (double)incorrect / tot_trials * 100);
    uint64_t time_per_run = tot_ns / tot_trials;
    printf("Time usage: %lu.%lu (us)\n", time_per_run / 1000, time_per_run % 1000);
    printf("over %d iterations.\n", tot_trials);
    return 0;
}
