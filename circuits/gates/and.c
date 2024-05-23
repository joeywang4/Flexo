#include <stdbool.h>
#include <stdio.h>

// compile with clang -O1 -fno-inline-functions -S -emit-llvm -o and.ll and.c

bool __weird__and_gate(bool in1, bool in2, bool* out) {
    out[0] = in1 & in2;
    return false;
}

int main() {
    int correct = 0;
    int detected_errors = 0;
    const int trials = 1000;

    for (int i = 0; i < trials; ++i) {
        bool in1 = i & 1;
        bool in2 = i & 2;
        bool out;
        bool error_detected = __weird__and_gate(in1, in2, &out);
        correct += (int)((in1 & in2) == out);
        detected_errors += (int)error_detected;
    }

    printf("Accuracy: %.2lf%%\n", (double)correct * 100 / trials);
    printf("Detected error: %.2lf%%\n", (double)detected_errors * 100 / trials);

    return 0;
}
