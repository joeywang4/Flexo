#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <argp.h>
#include "sha1.h"

#define BLOCKS 2

unsigned tot_trials = 1000;
bool retry = false;

bool __weird__sha1_round1(
    unsigned* input, unsigned w, unsigned* output, unsigned* error_output
) { SHA1_ROUND(input, w, output, 1); }

bool __weird__sha1_round2(
    unsigned* input, unsigned w, unsigned* output, unsigned* error_output
) { SHA1_ROUND(input, w, output, 2); }

bool __weird__sha1_round3(
    unsigned* input, unsigned w, unsigned* output, unsigned* error_output
) { SHA1_ROUND(input, w, output, 3); }

bool __weird__sha1_round4(
    unsigned* input, unsigned w, unsigned* output, unsigned* error_output
) { SHA1_ROUND(input, w, output, 4); }

bool sha1_block(unsigned* block, unsigned* states, bool do_ref) {
    bool has_error;
    unsigned w[80], new_states[SIZE], ori_states[SIZE], err_out[SIZE];
    char votes[SIZE * UNIT_SIZE] = {0};

    for (int i = 0; i < 16; ++i) {
        w[i] = block[i];
    }
    for (int i = 16; i < 80; ++i) {
        w[i] = ROL(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }

    for (int i = 0; i < 5; ++i) ori_states[i] = states[i];

    for (int i = 0; i < 80; ++i) {
        if (i <= 19) {
            if (do_ref) ref_sha1_round(states, w[i], new_states, 0);
            else RUN_SHA1_ROUND(1, w[i]);
        }
        else if (i <= 39) {
            if (do_ref) ref_sha1_round(states, w[i], new_states, 1);
            else RUN_SHA1_ROUND(2, w[i]);
        }
        else if (i <= 59) {
            if (do_ref) ref_sha1_round(states, w[i], new_states, 2);
            else RUN_SHA1_ROUND(3, w[i]);
        }
        else {
            if (do_ref) ref_sha1_round(states, w[i], new_states, 3);
            else RUN_SHA1_ROUND(4, w[i]);
        }

        for (int j = 0; j < 5; ++j) states[j] = new_states[j];
    }

    for (int i = 0; i < 5; ++i) states[i] += ori_states[i];
    return has_error;
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
    unsigned correct = 0, detected = 0, incorrect = 0;
    unsigned input[16 * BLOCKS], states[5], ref_states[5];

    for (unsigned i = 0; i < tot_trials; ++i) {
        unsigned j;

        // init states
        states[0] = ref_states[0] = 0x67452301;
        states[1] = ref_states[1] = 0xEFCDAB89;
        states[2] = ref_states[2] = 0x98BADCFE;
        states[3] = ref_states[3] = 0x10325476;
        states[4] = ref_states[4] = 0xC3D2E1F0;

        // generate input
        for (j = 0; j < (16 * (BLOCKS - 1)); ++j) {
            input[j] = rand();
        }
        unsigned last_size = rand() % 13;
        const unsigned offset = 16 * (BLOCKS - 1);
        for (j = 0; j < 14; ++j) {
            input[offset + j] = (j >= last_size) ? 0 : rand();
        }
        input[offset + last_size] = 0x80000000;
        uint64_t bit_size = (16 * (BLOCKS - 1) + last_size) * 32;
        input[offset + 14] = bit_size >> 32;
        input[offset + 15] = bit_size;

        #ifdef DEBUG
        printf("Input: ");
        for (j = 0; j < 16 * BLOCKS; ++j) {
            printf("%08X", input[j]);
        }
        printf("\n");
        #endif

        // calculate SHA1
        for (j = 0; j < BLOCKS; ++j) {
            sha1_block(&input[16 * j], ref_states, true);
        }
        bool has_error = false;
        struct timespec ts_start;
        struct timespec ts_end;
        clock_gettime(CLOCK_MONOTONIC, &ts_start);
        for (j = 0; j < BLOCKS; ++j) {
            has_error |= sha1_block(&input[16 * j], states, false);
        }
        clock_gettime(CLOCK_MONOTONIC, &ts_end);
        tot_ns += time_elasped(&ts_start, &ts_end);

        // compare results
        #ifdef DEBUG
        printf("Output: ");
        for (j = 0; j < 5; ++j) {
            printf("%08X", ref_states[j]);
        }
        printf("\n");
        #endif

        bool error = false;
        for (j = 0; j < 5 && states[j] == ref_states[j]; ++j);
        if (has_error) {
            ++detected;
        }
        else if (j < 5) {
            ++incorrect;
        }
        else {
            ++correct;
        }
    }

    printf("=== SHA1 2 Blocks ===\n");
    printf("Accuracy: %.5f%%, ", (double)correct / tot_trials * 100);
    printf("Error detected: %.5f%%, ", (double)detected / tot_trials * 100);
    printf("Undetected error: %.5f%%\n", (double)incorrect / tot_trials * 100);
    uint64_t time_per_run = tot_ns / tot_trials;
    printf("Time usage: %lu.%lu (us)\n", time_per_run / 1000, time_per_run % 1000);
    printf("over %d iterations.\n", tot_trials);
    return 0;
}
