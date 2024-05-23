#include <argp.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include "macro.h"

/* Config */
bool verbose = false;
unsigned tot_trials = 10000;

// compile with clang -O1 -fno-inline-functions -S -emit-llvm -o adder.ll adder.cpp

// control the test cases here
#define TEST2  0
#define TEST3  0
#define TEST4  0
#define TEST5  0
#define TEST6  0
#define TEST7  0
#define TEST8  1
#define TEST9  0
#define TEST10 0
#define TEST11 0
#define TEST12 0
#define TEST13 0
#define TEST14 0
#define TEST15 0
#define TEST16 1
#define TEST17 0
#define TEST18 0
#define TEST19 0
#define TEST20 0
#define TEST21 0
#define TEST22 0
#define TEST23 0
#define TEST24 0
#define TEST25 0
#define TEST26 0
#define TEST27 0
#define TEST28 0
#define TEST29 0
#define TEST30 0
#define TEST31 0
#define TEST32 1
#define TEST33 0
#define TEST34 0
#define TEST35 0
#define TEST36 0
#define TEST37 0
#define TEST38 0
#define TEST39 0
#define TEST40 0
#define TEST41 0
#define TEST42 0
#define TEST43 0
#define TEST44 0
#define TEST45 0
#define TEST46 0
#define TEST47 0
#define TEST48 0
#define TEST49 0
#define TEST50 0
#define TEST51 0
#define TEST52 0
#define TEST53 0
#define TEST54 0
#define TEST55 0
#define TEST56 0
#define TEST57 0
#define TEST58 0
#define TEST59 0
#define TEST60 0
#define TEST61 0
#define TEST62 0
#define TEST63 0
#define TEST64 0
// end test cases

#define WEIRD_ADDER(bits, bytes) \
void __weird__adder##bits(unsigned char* in1, unsigned char* in2, unsigned char* out, unsigned char* error_out) { \
    u_int64_t num1 = TO_LONG(in1); \
    MASK(num1, bits); \
    u_int64_t num2 = TO_LONG(in2); \
    MASK(num2, bits); \
    u_int64_t sum = num1 + num2; \
    MASK(sum, bits); \
    TO_BYTES##bytes(out, sum); \
}

#define TEST_ACC(bits) \
    test_acc(bits, __weird__adder##bits);

EXPR_IF(TEST2,  WEIRD_ADDER( 2, 1));
EXPR_IF(TEST3,  WEIRD_ADDER( 3, 1));
EXPR_IF(TEST4,  WEIRD_ADDER( 4, 1));
EXPR_IF(TEST5,  WEIRD_ADDER( 5, 1));
EXPR_IF(TEST6,  WEIRD_ADDER( 6, 1));
EXPR_IF(TEST7,  WEIRD_ADDER( 7, 1));
EXPR_IF(TEST8,  WEIRD_ADDER( 8, 1));
EXPR_IF(TEST9,  WEIRD_ADDER( 9, 2));
EXPR_IF(TEST10, WEIRD_ADDER(10, 2));
EXPR_IF(TEST11, WEIRD_ADDER(11, 2));
EXPR_IF(TEST12, WEIRD_ADDER(12, 2));
EXPR_IF(TEST13, WEIRD_ADDER(13, 2));
EXPR_IF(TEST14, WEIRD_ADDER(14, 2));
EXPR_IF(TEST15, WEIRD_ADDER(15, 2));
EXPR_IF(TEST16, WEIRD_ADDER(16, 2));
EXPR_IF(TEST17, WEIRD_ADDER(17, 3));
EXPR_IF(TEST18, WEIRD_ADDER(18, 3));
EXPR_IF(TEST19, WEIRD_ADDER(19, 3));
EXPR_IF(TEST20, WEIRD_ADDER(20, 3));
EXPR_IF(TEST21, WEIRD_ADDER(21, 3));
EXPR_IF(TEST22, WEIRD_ADDER(22, 3));
EXPR_IF(TEST23, WEIRD_ADDER(23, 3));
EXPR_IF(TEST24, WEIRD_ADDER(24, 3));
EXPR_IF(TEST25, WEIRD_ADDER(25, 4));
EXPR_IF(TEST26, WEIRD_ADDER(26, 4));
EXPR_IF(TEST27, WEIRD_ADDER(27, 4));
EXPR_IF(TEST28, WEIRD_ADDER(28, 4));
EXPR_IF(TEST29, WEIRD_ADDER(29, 4));
EXPR_IF(TEST30, WEIRD_ADDER(30, 4));
EXPR_IF(TEST31, WEIRD_ADDER(31, 4));
EXPR_IF(TEST32, WEIRD_ADDER(32, 4));
EXPR_IF(TEST33, WEIRD_ADDER(33, 5));
EXPR_IF(TEST34, WEIRD_ADDER(34, 5));
EXPR_IF(TEST35, WEIRD_ADDER(35, 5));
EXPR_IF(TEST36, WEIRD_ADDER(36, 5));
EXPR_IF(TEST37, WEIRD_ADDER(37, 5));
EXPR_IF(TEST38, WEIRD_ADDER(38, 5));
EXPR_IF(TEST39, WEIRD_ADDER(39, 5));
EXPR_IF(TEST40, WEIRD_ADDER(40, 5));
EXPR_IF(TEST41, WEIRD_ADDER(41, 6));
EXPR_IF(TEST42, WEIRD_ADDER(42, 6));
EXPR_IF(TEST43, WEIRD_ADDER(43, 6));
EXPR_IF(TEST44, WEIRD_ADDER(44, 6));
EXPR_IF(TEST45, WEIRD_ADDER(45, 6));
EXPR_IF(TEST46, WEIRD_ADDER(46, 6));
EXPR_IF(TEST47, WEIRD_ADDER(47, 6));
EXPR_IF(TEST48, WEIRD_ADDER(48, 6));
EXPR_IF(TEST49, WEIRD_ADDER(49, 7));
EXPR_IF(TEST50, WEIRD_ADDER(50, 7));
EXPR_IF(TEST51, WEIRD_ADDER(51, 7));
EXPR_IF(TEST52, WEIRD_ADDER(52, 7));
EXPR_IF(TEST53, WEIRD_ADDER(53, 7));
EXPR_IF(TEST54, WEIRD_ADDER(54, 7));
EXPR_IF(TEST55, WEIRD_ADDER(55, 7));
EXPR_IF(TEST56, WEIRD_ADDER(56, 7));
EXPR_IF(TEST57, WEIRD_ADDER(57, 8));
EXPR_IF(TEST58, WEIRD_ADDER(58, 8));
EXPR_IF(TEST59, WEIRD_ADDER(59, 8));
EXPR_IF(TEST60, WEIRD_ADDER(60, 8));
EXPR_IF(TEST61, WEIRD_ADDER(61, 8));
EXPR_IF(TEST62, WEIRD_ADDER(62, 8));
EXPR_IF(TEST63, WEIRD_ADDER(63, 8));
EXPR_IF(TEST64, WEIRD_ADDER(64, 8));

/* Test the accuracy and time usage of an adder */
void test_acc(
    unsigned input_size,
    void (*gate_fn)(unsigned char*, unsigned char*, unsigned char*, unsigned char*)
) {
    const unsigned in_space = input_size <= 8 ? 1 << (input_size << 1) : 0;
    unsigned tot_correct_counts = 0, tot_detected_counts = 0, tot_error_counts = 0;
    std::vector<unsigned> all0_errors(input_size * in_space, 0);
    std::vector<unsigned> all1_errors(input_size * in_space, 0);
    std::vector<unsigned> bit_error_counts(input_size * in_space, 0);
    clock_t end_t, start_t = clock();

    for (unsigned seed = 0; seed < tot_trials; seed++) {
        unsigned char in1[8], in2[8], out[8] = {0}, error_out[8] = {0};
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
        gate_fn(in1, in2, out, error_out);

        u_int64_t expected = num1 + num2;
        MASK(expected, input_size);
        u_int64_t result = TO_LONG(out);
        MASK(result, input_size);
        u_int64_t detected = TO_LONG(error_out);
        MASK(detected, input_size);

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
        for (unsigned bit = 0; bit < input_size; ++bit) {
            bool bit_expected = (expected >> bit) & 1;
            bool bit_result = (result >> bit) & 1;
            if (detected & (1 << bit)) {
                if (bit_result) {
                    ++all1_errors[(seed % in_space) * input_size + bit];
                }
                else {
                    ++all0_errors[(seed % in_space) * input_size + bit];
                }
            }
            else if (bit_result != bit_expected) {
                ++bit_error_counts[(seed % in_space) * input_size + bit];
            }
        }
    }

    end_t = clock();

    printf("=== ADDER %u ===\n", input_size);
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
        printf("%u + %u:", num1, num2);

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
    EXPR_IF(TEST17, TEST_ACC(17));
    EXPR_IF(TEST18, TEST_ACC(18));
    EXPR_IF(TEST19, TEST_ACC(19));
    EXPR_IF(TEST20, TEST_ACC(20));
    EXPR_IF(TEST21, TEST_ACC(21));
    EXPR_IF(TEST22, TEST_ACC(22));
    EXPR_IF(TEST23, TEST_ACC(23));
    EXPR_IF(TEST24, TEST_ACC(24));
    EXPR_IF(TEST25, TEST_ACC(25));
    EXPR_IF(TEST26, TEST_ACC(26));
    EXPR_IF(TEST27, TEST_ACC(27));
    EXPR_IF(TEST28, TEST_ACC(28));
    EXPR_IF(TEST29, TEST_ACC(29));
    EXPR_IF(TEST30, TEST_ACC(30));
    EXPR_IF(TEST31, TEST_ACC(31));
    EXPR_IF(TEST32, TEST_ACC(32));
    EXPR_IF(TEST33, TEST_ACC(33));
    EXPR_IF(TEST34, TEST_ACC(34));
    EXPR_IF(TEST35, TEST_ACC(35));
    EXPR_IF(TEST36, TEST_ACC(36));
    EXPR_IF(TEST37, TEST_ACC(37));
    EXPR_IF(TEST38, TEST_ACC(38));
    EXPR_IF(TEST39, TEST_ACC(39));
    EXPR_IF(TEST40, TEST_ACC(40));
    EXPR_IF(TEST41, TEST_ACC(41));
    EXPR_IF(TEST42, TEST_ACC(42));
    EXPR_IF(TEST43, TEST_ACC(43));
    EXPR_IF(TEST44, TEST_ACC(44));
    EXPR_IF(TEST45, TEST_ACC(45));
    EXPR_IF(TEST46, TEST_ACC(46));
    EXPR_IF(TEST47, TEST_ACC(47));
    EXPR_IF(TEST48, TEST_ACC(48));
    EXPR_IF(TEST49, TEST_ACC(49));
    EXPR_IF(TEST50, TEST_ACC(50));
    EXPR_IF(TEST51, TEST_ACC(51));
    EXPR_IF(TEST52, TEST_ACC(52));
    EXPR_IF(TEST53, TEST_ACC(53));
    EXPR_IF(TEST54, TEST_ACC(54));
    EXPR_IF(TEST55, TEST_ACC(55));
    EXPR_IF(TEST56, TEST_ACC(56));
    EXPR_IF(TEST57, TEST_ACC(57));
    EXPR_IF(TEST58, TEST_ACC(58));
    EXPR_IF(TEST59, TEST_ACC(59));
    EXPR_IF(TEST60, TEST_ACC(60));
    EXPR_IF(TEST61, TEST_ACC(61));
    EXPR_IF(TEST62, TEST_ACC(62));
    EXPR_IF(TEST63, TEST_ACC(63));
    EXPR_IF(TEST64, TEST_ACC(64));
}
