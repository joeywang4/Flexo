#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <argp.h>
#include <time.h>
#include <vector>

/* Config */
// bool verbose = false;
unsigned tot_trials = 100000;

// compile with clang -O1 -fno-inline-functions -S -emit-llvm -o test.ll test.cpp

uint64_t time_elasped(struct timespec* begin, struct timespec* end) {
    int64_t output = end->tv_sec - begin->tv_sec;
    output *= 1000000000;
    output += end->tv_nsec - begin->tv_nsec;
    return output;
}

bool __weird__and(bool in1, bool in2, bool& out) {
    out = in1 & in2;
    return out;
}

bool __weird__or(bool in1, bool in2, bool& out) {
    out = in1 | in2;
    return out;
}

bool __weird__not(bool in1, bool& out) {
    out = !in1;
    return out;
}

bool __weird__nand(bool in1, bool in2, bool& out) {
    out = !(in1 & in2);
    return out;
}

bool __weird__xor(bool in1, bool in2, bool& out) {
    out = in1 ^ in2;
    return out;
}

bool __weird__mux(bool in1, bool in2, bool in3, bool& out) {
    out = ((in1 & !(in3)) | (in2 & in3));
    return out;
}

bool __weird__xor3(bool in1, bool in2, bool in3, bool& out) {
    out = in1 ^ in2 ^ in3;
    return out;
}

bool __weird__xor4(bool in1, bool in2, bool in3, bool in4, bool& out) {
    out = in1 ^ in2 ^ in3 ^ in4;
    return out;
}

/* Test the accuracy and time usage of a gate */
void test_acc(
    const char* name,
    unsigned input_size,
    unsigned (*gate_fn)(unsigned)
) {
    const unsigned in_space = 1 << input_size;
    unsigned tot_correct_counts = 0, tot_detected_counts = 0, tot_error_counts = 0;
    unsigned all0_errors = 0, all1_errors = 0;
    struct timespec ts_start;
    struct timespec ts_end;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    for (unsigned seed = 0; seed < tot_trials; seed++) {
        unsigned result = gate_fn(seed);

        if (result == 0) {
            ++tot_correct_counts;
        }
        else if (result & 2) {
            ++tot_detected_counts;
        }
        else {
            ++tot_error_counts;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    uint64_t tot_ns = time_elasped(&ts_start, &ts_end);

    printf("=== %s gate ===\n", name);
    printf("Accuracy: %.5f%%, ", (double)tot_correct_counts / tot_trials * 100);
    printf("Error detected: %.5f%%, ", (double)tot_detected_counts / tot_trials * 100);
    printf("Undetected error: %.5f%%\n", (double)tot_error_counts / tot_trials * 100);
    uint64_t time_per_run = tot_ns / tot_trials;
    printf("Time usage: %lu.%lu (us)\n", time_per_run / 1000, time_per_run % 1000);
    printf("over %d iterations.\n", tot_trials);
}

/* Arguments */
static char doc[] = "Test the accuracy and run time of weird machines.";
static char args_doc[] = "";
static struct argp_option options[] = { 
    { "verbose", 'v', 0, 0, "Produce verbose output"},
    { "trial", 't', "TRIAL", 0, "Number of trials (default: 100000)."},
    { 0 } 
};
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    switch (key) {
        // case 'v': verbose = true; break;
        case 't': tot_trials = atoi(arg); break;
        case ARGP_KEY_ARG: return 0;
        default: return ARGP_ERR_UNKNOWN;
    }   
    return 0;
}
static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

int main(int argc, char* argv[]) {
    argp_parse(&argp, argc, argv, 0, 0, 0);

    test_acc("AND", 2, [](unsigned in) { 
        bool out;
        bool errorDetected = __weird__and(in & 1, (in & 2) >> 1, out);
        unsigned result = (errorDetected << 1) | (((in & 3) == 3) != out);
        return result;
    });
    test_acc("OR", 2, [](unsigned in) {
        bool out;
        bool errorDetected = __weird__or(in & 1, (in & 2) >> 1, out);
        unsigned result = (errorDetected << 1) | (((in & 3) != 0) != out);
        return result;
    });
    test_acc("NOT", 1, [](unsigned in) {
        bool out;
        bool errorDetected = __weird__not(in & 1, out);
        unsigned result = (errorDetected << 1) | (((in & 1) == 0) != out);
        return result;
    });
    test_acc("NAND", 2, [](unsigned in) { 
        bool out;
        bool errorDetected = __weird__nand(in & 1, (in & 2) >> 1, out);
        unsigned result = (errorDetected << 1) | (((in & 3) != 3) != out);
        return result;
    });
    test_acc("XOR", 2, [](unsigned in) {
        bool out;
        bool errorDetected = __weird__xor(in & 1, (in & 2) >> 1, out);
        unsigned result = (errorDetected << 1);
        result |= ((in & 3) == 1 | (in & 3) == 2) != out;
        return result;
    });
    test_acc("MUX", 3, [](unsigned in) {
        bool in1 = in & 1;
        bool in2 = in & 2;
        bool in3 = in & 4;
        bool out;
        bool errorDetected = __weird__mux(in1, in2, in3, out);
        unsigned result = (errorDetected << 1);
        result |= ((in1 & !(in3)) | (in2 & in3)) != out;
        return result;
    });
    test_acc("XOR3", 3, [](unsigned in) {
        bool in1 = in & 1;
        bool in2 = in & 2;
        bool in3 = in & 4;
        bool out;
        bool errorDetected = __weird__xor3(in1, in2, in3, out);
        unsigned result = (errorDetected << 1);
        result |= (in1 ^ in2 ^ in3) != out;
        return result;
    });
    test_acc("XOR4", 4, [](unsigned in) {
        bool in1 = in & 1;
        bool in2 = in & 2;
        bool in3 = in & 4;
        bool in4 = in & 8;
        bool out;
        bool errorDetected = __weird__xor4(in1, in2, in3, in4, out);
        unsigned result = (errorDetected << 1);
        result |= (in1 ^ in2 ^ in3 ^ in4) != out;
        return result;
    });
}
