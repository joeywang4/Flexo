#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <argp.h>
#include <time.h>
#include <vector>

#define THRESHOLD 2
#define SIZE 6

typedef unsigned char byte;

/* Config */
unsigned tot_trials = 1000000;
bool retry = false;

uint64_t time_elasped(struct timespec* begin, struct timespec* end) {
    int64_t output = end->tv_sec - begin->tv_sec;
    output *= 1000000000;
    output += end->tv_nsec - begin->tv_nsec;
    return output;
}

// reference ALU as ground truth
// x, y: 4 bits
// control: 6 bits - zx, nx, zy, ny, f, no
// out: 6 bits - out1, out2, out3, out4, zr, ng
void ref_alu(byte* x, byte* y, byte* control, byte* out) {
    u_int8_t x_int = x[0];
    u_int8_t y_int = y[0];
    u_int8_t out_int = 0;

    // zx
    if (control[0] & 1) { x_int = 0; }
    // nx
    if (control[0] & 2) { x_int = ~x_int; }
    // zy
    if (control[0] & 4) { y_int = 0; }
    // ny
    if (control[0] & 8) { y_int = ~y_int; }
    // f
    if (control[0] & 0x10) { out_int = x_int + y_int; }
    else { out_int = x_int & y_int; }
    // no
    if (control[0] & 0x20) { out_int = ~out_int; }

    out_int &= 0xF;
    // out_int -> out[0:3]
    out[0] = out_int;

    // zr
    if (out_int == 0) { out[0] |= 0x10; }
    // ng
    if (out[0] & 8) { out[0] |= 0x20; }
}

bool __weird__alu(
    byte* x, byte* y, byte* control, byte* out, byte* error_out
) { return false; }

bool update_votes(byte* output, byte* error_output, char* votes) {
    bool all_done = true;
    for (int j = 0; j < SIZE; ++j) {
        bool out_bit = (output[0] >> j) & 1;
        bool err_bit = (error_output[0] >> j) & 1;

        if (!err_bit) {
            if (out_bit && votes[j] < 126) ++votes[j];
            else if (!out_bit && votes[j] > -126) --votes[j];
        }
        if (votes[j] < THRESHOLD && votes[j] > -THRESHOLD) all_done = false;
    }
    return all_done;
}

void merge_output(byte* output, char* votes) {
    for (unsigned j = 0; j < SIZE; ++j) {
        if (votes[j] >= THRESHOLD) output[0] |= 1 << j;
        else if (votes[j] <= -THRESHOLD) output[0] &= ~(1 << j);
    }
}

/* Test accuracy and time usage */
void test_acc(
    bool (*gate_fn)(byte*, byte*, byte*, byte*, byte*)
) {
    unsigned tot_correct_counts = 0, tot_detected_counts = 0, tot_error_counts = 0;
    uint64_t tot_ns = 0;

    for (unsigned seed = 0; seed < tot_trials; seed++) {
        // inputs and outputs
        byte x[1], y[1], control[1], out[1] = {0}, err_out[1] = {0};
        byte x_ref[1], y_ref[1], control_ref[1], out_ref[1] = {0};
        char votes[6] = {0};
        bool has_error;

        x_ref[0] = x[0] = seed & ((1 << 4) - 1);
        y_ref[0] = y[0] = (seed >> 4) & ((1 << 4) - 1);
        control_ref[0] = control[0] = (seed >> 8) & ((1 << 6) - 1);

        // execute the circuit
        struct timespec ts_start;
        struct timespec ts_end;
        clock_gettime(CLOCK_MONOTONIC, &ts_start);
        do {
            has_error = gate_fn(x, y, control, out, err_out);
            if (update_votes(out, err_out, votes)) has_error = false;
        } while (retry && has_error);
        merge_output(out, votes);
        clock_gettime(CLOCK_MONOTONIC, &ts_end);
        tot_ns += time_elasped(&ts_start, &ts_end);

        // execute the reference ALU
        ref_alu(x_ref, y_ref, control_ref, out_ref);
        byte expected = out_ref[0] & 0x3F;
        byte result = out[0] & 0x3F;

        if (has_error) {
            ++tot_detected_counts;
        }
        else if (expected == result) {
            ++tot_correct_counts;
        }
        else {
            ++tot_error_counts;
        }
    }

    uint64_t time_per_run = tot_ns / tot_trials;

    printf("=== ALU ===\n");
    printf("Accuracy: %.5f%%, ", (double)tot_correct_counts / tot_trials * 100);
    printf("Error detected: %.5f%%, ", (double)tot_detected_counts / tot_trials * 100);
    printf("Undetected error: %.5f%%\n", (double)tot_error_counts / tot_trials * 100);
    printf("Time usage: %lu.%lu (us)\n", time_per_run / 1000, time_per_run % 1000);
    printf("over %d iterations.\n", tot_trials);
}

/* Arguments */
static char doc[] = "Test the accuracy and run time of weird machines.";
static char args_doc[] = "";
static struct argp_option options[] = {
    { "retry", 'r', 0, 0, "Retry when an error is detected"},
    { "trial", 't', "TRIAL", 0, "Number of trials."},
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
    test_acc(__weird__alu);
}
