#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "aes.h"

#define memcpy(a, b, c) \
    for (int i = 0; i < c; ++i) b[i] = a[i];

unsigned tot_trials = 100;
bool retry = false;

inline __attribute__((always_inline)) byte sbox(byte input) {
    SBOX_IMPL(input);
    return out;
}

bool __weird__aes_round(byte* input, byte* key, byte* output, byte* error_output) {
    // prevent clang's optimization on error_output and the return value
    error_output[0] = input[0];

    Sbox_ShiftRows(input, sbox);
    MixColumns_RoundKey(key, output);
}

bool __weird__aes_first_round(
    byte* input, byte* key, byte* round_key, byte* output, byte* error_output
) {
    // prevent clang's optimization on error_output and the return value
    error_output[0] = input[0];

    /* SubBytes + ShiftRows */
    byte shifted_0 = sbox(key[0] ^ input[0]);
    byte shifted_1 = sbox(key[5] ^ input[5]);
    byte shifted_2 = sbox(key[10] ^ input[10]);
    byte shifted_3 = sbox(key[15] ^ input[15]);
    byte shifted_4 = sbox(key[4] ^ input[4]);
    byte shifted_5 = sbox(key[9] ^ input[9]);
    byte shifted_6 = sbox(key[14] ^ input[14]);
    byte shifted_7 = sbox(key[3] ^ input[3]);
    byte shifted_8 = sbox(key[8] ^ input[8]);
    byte shifted_9 = sbox(key[13] ^ input[13]);
    byte shifted_A = sbox(key[2] ^ input[2]);
    byte shifted_B = sbox(key[7] ^ input[7]);
    byte shifted_C = sbox(key[12] ^ input[12]);
    byte shifted_D = sbox(key[1] ^ input[1]);
    byte shifted_E = sbox(key[6] ^ input[6]);
    byte shifted_F = sbox(key[11] ^ input[11]);

    MixColumns_RoundKey(round_key, output);
}

bool __weird__aes_last_round(
    byte* input, byte* key, byte rcon, byte* output, byte* error_output
) {
    // last round key
    byte t_0 = sbox(key[13]) ^ rcon;
    byte t_1 = sbox(key[14]);
    byte t_2 = sbox(key[15]);
    byte t_3 = sbox(key[12]);

    byte next_key_0 = key[0] ^ t_0;
    byte next_key_1 = key[1] ^ t_1;
    byte next_key_2 = key[2] ^ t_2;
    byte next_key_3 = key[3] ^ t_3;

    /* SubBytes + ShiftRows + AddRoundKey */
    output[0] = next_key_0 ^ sbox(input[0]);
    output[1] = next_key_1 ^ sbox(input[5]);
    output[2] = next_key_2 ^ sbox(input[10]);
    output[3] = next_key_3 ^ sbox(input[15]);

    byte next_key_4 = key[4] ^ next_key_0;
    byte next_key_5 = key[5] ^ next_key_1;
    byte next_key_6 = key[6] ^ next_key_2;
    byte next_key_7 = key[7] ^ next_key_3;

    output[4] = next_key_4 ^ sbox(input[4]);
    output[5] = next_key_5 ^ sbox(input[9]);
    output[6] = next_key_6 ^ sbox(input[14]);
    output[7] = next_key_7 ^ sbox(input[3]);

    byte next_key_8 = key[8] ^ next_key_4;
    byte next_key_9 = key[9] ^ next_key_5;
    byte next_key_10 = key[10] ^ next_key_6;
    byte next_key_11 = key[11] ^ next_key_7;

    output[8] = next_key_8 ^ sbox(input[8]);
    output[9] = next_key_9 ^ sbox(input[13]);
    output[10] = next_key_10 ^ sbox(input[2]);
    output[11] = next_key_11 ^ sbox(input[7]);

    output[12] = next_key_8 ^ key[12] ^ sbox(input[12]);
    output[13] = next_key_9 ^ key[13] ^ sbox(input[1]);
    output[14] = next_key_10 ^ key[14] ^ sbox(input[6]);
    output[15] = next_key_11 ^ key[15] ^ sbox(input[11]);

    // prevent clang's optimization on error_output and the return value
    error_output[0] = input[0];
    return error_output[0] & 1;
}

bool __weird__round_key(
    byte* prev_key, byte* next_key, byte rcon, byte* error_next_key
) {
    // prevent clang's optimization on error_output and the return value
    error_next_key[0] = prev_key[0];

    byte t_0 = sbox(prev_key[13]) ^ rcon;
    byte t_1 = sbox(prev_key[14]);
    byte t_2 = sbox(prev_key[15]);
    byte t_3 = sbox(prev_key[12]);

    next_key[0] = prev_key[0] ^ t_0;
    next_key[1] = prev_key[1] ^ t_1;
    next_key[2] = prev_key[2] ^ t_2;
    next_key[3] = prev_key[3] ^ t_3;

    next_key[4] = prev_key[4] ^ next_key[0];
    next_key[5] = prev_key[5] ^ next_key[1];
    next_key[6] = prev_key[6] ^ next_key[2];
    next_key[7] = prev_key[7] ^ next_key[3];

    next_key[8] = prev_key[8] ^ next_key[4];
    next_key[9] = prev_key[9] ^ next_key[5];
    next_key[10] = prev_key[10] ^ next_key[6];
    next_key[11] = prev_key[11] ^ next_key[7];

    next_key[12] = prev_key[12] ^ next_key[8];
    next_key[13] = prev_key[13] ^ next_key[9];
    next_key[14] = prev_key[14] ^ next_key[10];
    next_key[15] = prev_key[15] ^ next_key[11];
    
    return next_key[0];
}

bool aes_encrypt(byte* input, byte* key, byte* output) {
    byte round_key[16], err_out[16];
    char votes[SIZE] = {0};
    bool has_error = false;

    // first round
    do {
        has_error = __weird__round_key(key, round_key, rcon[1], err_out);
        if (update_votes(round_key, err_out, votes)) has_error = false;
    } while (retry && has_error);
    merge_output(round_key, votes);

    do {
        has_error = __weird__aes_first_round(input, key, round_key, output, err_out);
        if (update_votes(output, err_out, votes)) has_error = false;
    } while (retry && has_error);
    merge_output(output, votes);

    memcpy(round_key, key, 16);
    memcpy(output, input, 16);

    for (unsigned i = 2; i < 10; ++i) {
        do {
            has_error = __weird__round_key(key, round_key, rcon[i], err_out);
            if (update_votes(round_key, err_out, votes)) has_error = false;
        } while (retry && has_error);
        merge_output(round_key, votes);
        memcpy(round_key, key, 16);

        do {
            has_error = __weird__aes_round(input, round_key, output, err_out);
            if (update_votes(output, err_out, votes)) has_error = false;
        } while (retry && has_error);
        merge_output(output, votes);
        memcpy(output, input, 16);
    }

    do {
        has_error = __weird__aes_last_round(input, key, rcon[10], output, err_out);
        if (update_votes(output, err_out, votes)) has_error = false;
    } while (retry && has_error);
    merge_output(output, votes);

    return has_error;
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
    byte input[16], key[16], output[16] = {0}, ref[16] = {0};

    int correct = 0, detected = 0, incorrect = 0;

    for (unsigned i = 0; i < tot_trials; ++i) {
        for (unsigned j = 0; j < 4; ++j) {
            ((unsigned*)input)[j] = rand();
            ((unsigned*)key)[j] = rand();
        }
        ref_aes_encrypt(input, key, ref);

        // execute the circuit
        struct timespec ts_start;
        struct timespec ts_end;
        clock_gettime(CLOCK_MONOTONIC, &ts_start);
        bool has_error = aes_encrypt(input, key, output);
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

    printf("=== AES Encryption (1 block) ===\n");
    printf("Accuracy: %.5f%%, ", (double)correct / tot_trials * 100);
    printf("Error detected: %.5f%%, ", (double)detected / tot_trials * 100);
    printf("Undetected error: %.5f%%\n", (double)incorrect / tot_trials * 100);
    uint64_t time_per_run = tot_ns / tot_trials;
    printf("Time usage: %lu.%lu (us)\n", time_per_run / 1000, time_per_run % 1000);
    printf("over %d iterations.\n", tot_trials);
    return 0;
}
