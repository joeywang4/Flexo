#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <argp.h>
#include "sha1.h"

unsigned tot_trials = 1000;
bool retry = false;

bool __weird__sha1_round(
    unsigned* input, unsigned w, unsigned* output, unsigned* error_output
) { SHA1_ROUND(input, w, output, 1); }

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
    unsigned correct = 0, detected = 0, incorrect = 0, w;
    unsigned input[SIZE], output[SIZE], err_out[SIZE], ref_output[SIZE];
    char votes[SIZE * UNIT_SIZE] = {0};
    bool has_error;

    for (unsigned i = 0; i < tot_trials; ++i) {
        unsigned j = 0;
        for (; j < SIZE; ++j) input[j] = rand();
        w = rand();

        // reference implementation (ground truth)
        ref_sha1_round(input, w, ref_output, 0);

        // execute SHA1 circuit for the first round
        struct timespec ts_start;
        struct timespec ts_end;
        clock_gettime(CLOCK_MONOTONIC, &ts_start);
        do {
            has_error = __weird__sha1_round(input, w, output, err_out);
            if (update_votes(output, err_out, votes)) has_error = false;
        } while (retry && has_error);
        merge_output(output, votes);
        clock_gettime(CLOCK_MONOTONIC, &ts_end);
        tot_ns += time_elasped(&ts_start, &ts_end);

        bool error = false;
        for (j = 0; j < SIZE && output[j] == ref_output[j]; ++j);
        if (has_error) {
            ++detected;
        }
        else if (j < SIZE) {
            ++incorrect;
        }
        else {
            ++correct;
        }
    }

    printf("=== SHA1 Round ===\n");
    printf("Accuracy: %.5f%%, ", (double)correct / tot_trials * 100);
    printf("Error detected: %.5f%%, ", (double)detected / tot_trials * 100);
    printf("Undetected error: %.5f%%\n", (double)incorrect / tot_trials * 100);
    uint64_t time_per_run = tot_ns / tot_trials;
    printf("Time usage: %lu.%lu (us)\n", time_per_run / 1000, time_per_run % 1000);
    printf("over %d iterations.\n", tot_trials);
    return 0;
}
