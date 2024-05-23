#include <stdio.h>
#include <math.h>
#include <argp.h>
#include <time.h>
#include <vector>
#include "macro.h"

/* Config */
bool verbose = false;
unsigned tot_trials = 10000;

// compile with clang -O1 -fno-inline-functions -S -emit-llvm -o mul.ll mul.cpp

// control the test cases here
#define TEST2  1
#define TEST3  1
#define TEST4  1
#define TEST5  1
#define TEST6  1
#define TEST7  1
#define TEST8  1
#define TEST9  1
#define TEST10 1
#define TEST11 1
#define TEST12 1
#define TEST13 1
#define TEST14 1
#define TEST15 1
#define TEST16 1
// end test cases

#define WEIRD_MUL(bits, bytes) \
void __weird__mul##bits(unsigned char* in1, unsigned char* in2, unsigned char* out, unsigned char* error_out) { \
    u_int64_t num1 = TO_LONG(in1); \
    MASK(num1, bits); \
    u_int64_t num2 = TO_LONG(in2); \
    MASK(num2, bits); \
    u_int64_t sum = num1 * num2; \
    MASK(sum, 2 * bits); \
    TO_BYTES##bytes(out, sum); \
}

#define TEST_ACC(bits) \
    test_acc(bits, __weird__mul##bits);

EXPR_IF(TEST2,  WEIRD_MUL( 2, 1));
EXPR_IF(TEST3,  WEIRD_MUL( 3, 1));
EXPR_IF(TEST4,  WEIRD_MUL( 4, 1));
EXPR_IF(TEST5,  WEIRD_MUL( 5, 2));
EXPR_IF(TEST6,  WEIRD_MUL( 6, 2));
EXPR_IF(TEST7,  WEIRD_MUL( 7, 2));
EXPR_IF(TEST8,  WEIRD_MUL( 8, 2));
EXPR_IF(TEST9,  WEIRD_MUL( 9, 3));
EXPR_IF(TEST10, WEIRD_MUL(10, 3));
EXPR_IF(TEST11, WEIRD_MUL(11, 3));
EXPR_IF(TEST12, WEIRD_MUL(12, 3));
EXPR_IF(TEST13, WEIRD_MUL(13, 4));
EXPR_IF(TEST14, WEIRD_MUL(14, 4));
EXPR_IF(TEST15, WEIRD_MUL(15, 4));
EXPR_IF(TEST16, WEIRD_MUL(16, 4));


/* Test the accuracy and time usage of an adder */
void test_acc(
    u_int64_t input_size,
    void (*gate_fn)(unsigned char*, unsigned char*, unsigned char*, unsigned char*)
) {
    const unsigned in_space = input_size <= 8 ? 1 << (input_size << 1) : 0;
    unsigned tot_correct_counts = 0, tot_detected_counts = 0, tot_error_counts = 0;
    std::vector<unsigned> all0_errors(input_size * 2 * in_space, 0);
    std::vector<unsigned> all1_errors(input_size * 2 * in_space, 0);
    std::vector<unsigned> bit_error_counts(input_size * 2 * in_space, 0);
    clock_t end_t, start_t = clock();

    for (unsigned seed = 0; seed < tot_trials; seed++) {
        unsigned char in1[8], in2[8], out[8] = {0}, err[8] = {0};
        u_int64_t num1 = seed;
        u_int64_t num2 = seed >> input_size;

        if (in_space == 0) {
            num1 = RAND64();
            num2 = RAND64();
        }
        MASK(num1, input_size);
        MASK(num2, input_size);
        TO_BYTES8(in1, num1);
        TO_BYTES8(in2, num2);
        gate_fn(in1, in2, out, err);

        const int outputBits = input_size * 2;
        u_int64_t expected = num1 * num2;
        MASK(expected, outputBits);
        u_int64_t result = TO_LONG(out);
        MASK(result, outputBits);
        unsigned detected = TO_LONG(err);
        MASK(detected, outputBits);

        if (detected) {
            ++tot_detected_counts;
        }
        else if (result == expected) {
            ++tot_correct_counts;
        }
        else {
            ++tot_error_counts;
        }

        if (in_space == 0) continue;
        for (unsigned bit = 0; bit < input_size * 2; ++bit) {
            bool bit_expected = (expected >> bit) & 1;
            bool bit_result = (result >> bit) & 1;
            if (detected & (1 << bit)) {
                if (bit_result) {
                    ++all1_errors[(seed % in_space) * input_size * 2 + bit];
                }
                else {
                    ++all0_errors[(seed % in_space) * input_size * 2 + bit];
                }
            }
            else if (bit_result != bit_expected) {
                ++bit_error_counts[(seed % in_space) * input_size * 2 + bit];
            }
        }
    }

    end_t = clock();

    printf("=== MULTIPLIER %lu ===\n", input_size);
    printf("Accuracy: %.5f%%, ", (double)tot_correct_counts / tot_trials * 100);
    printf("Error detected: %.5f%%, ", (double)tot_detected_counts / tot_trials * 100);
    printf("Undetected error: %.5f%%\n", (double)tot_error_counts / tot_trials * 100);
    printf("Time usage: %.5fs ", (double)(end_t - start_t) / CLOCKS_PER_SEC);
    printf("over %d iterations.\n", tot_trials);

    if (!verbose) return;

    printf("=== Per bit error rate (all-0, all-1, undetected error) ===\n");
    for (unsigned seed = 0; seed < in_space; ++seed) {
        unsigned num1 = seed & (1 << input_size) - 1;
        unsigned num2 = (seed >> input_size) & ((1 << input_size) - 1);
        printf("%u * %u:", num1, num2);

        for (unsigned bit = 0; bit < input_size; ++bit) {
            printf(" bit-%u(%4.2f%%, %4.2f%%, %4.2f%%)", bit,
                (double)all0_errors[bit + seed * input_size] / tot_trials * 100,
                (double)all1_errors[bit + seed * input_size] / tot_trials * 100,
                (double)bit_error_counts[bit + seed * input_size] / tot_trials * 100
            );
        }
        printf("\n");
    }
}

/* Arguments */
static char doc[] = "Test the accuracy and run time of weird machines.";
static char args_doc[] = "";
static struct argp_option options[] = { 
    { "verbose", 'v', 0, 0, "Produce verbose output"},
    { "trial", 't', "TRIAL", 0, "Number of trials (default: 10000)."},
    { 0 } 
};
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    switch (key) {
        case 'v': verbose = true; break;
        case 't': tot_trials = atoi(arg); break;
        case ARGP_KEY_ARG: return 0;
        default: return ARGP_ERR_UNKNOWN;
    }   
    return 0;
}
static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

int main(int argc, char* argv[]) {
    argp_parse(&argp, argc, argv, 0, 0, 0);

    EXPR_IF(TEST2,  TEST_ACC( 2));
    EXPR_IF(TEST3,  TEST_ACC( 3));
    EXPR_IF(TEST4,  TEST_ACC( 4));
    EXPR_IF(TEST5,  TEST_ACC( 5));
    EXPR_IF(TEST6,  TEST_ACC( 6));
    EXPR_IF(TEST7,  TEST_ACC( 7));
    EXPR_IF(TEST8,  TEST_ACC( 8));
    EXPR_IF(TEST9,  TEST_ACC( 9));
    EXPR_IF(TEST10, TEST_ACC(10));
    EXPR_IF(TEST11, TEST_ACC(11));
    EXPR_IF(TEST12, TEST_ACC(12));
    EXPR_IF(TEST13, TEST_ACC(13));
    EXPR_IF(TEST14, TEST_ACC(14));
    EXPR_IF(TEST15, TEST_ACC(15));
    EXPR_IF(TEST16, TEST_ACC(16));
}
