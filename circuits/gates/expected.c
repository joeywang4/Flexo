#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

// compile with gcc -O1 -o expected.elf expected.c
// compile with clang -O1 -o expected-clang.elf expected.c
// compile with clang -O1 -fno-inline-functions -S -emit-llvm -o expected.ll expected.c

#define NOP "0x90"
#define NOP4 "0x90,0x90,0x90,0x90"
#define NOP16 NOP4 "," NOP4 "," NOP4 "," NOP4
#define NOP64 NOP16 "," NOP16 "," NOP16 "," NOP16
#define NOP256 NOP64 "," NOP64 "," NOP64 "," NOP64
#define NOPs(data) asm volatile(".byte " data ::: "memory");

void mod_ret_addr();
asm(
    ".align 16\n"
    "mod_ret_addr:\n"
    // "    clflush (%rsp)\n"
    // begin div delay
    "    mov  $17789368654632150994, %rcx\n"
    "    mov  $2376950929536801796, %rax\n"
    "    mov  $5442431035597935919, %rdx\n"

    "    div  %rcx\n"
    "    div  %rcx\n"
    "    div  %rcx\n"
    "    div  %rcx\n"
    "    div  %rcx\n" // 5

    "    div  %rcx\n"
    "    div  %rcx\n"
    "    div  %rcx\n"
    "    div  %rcx\n"
    "    div  %rcx\n" // 10
    // end div delay

    "    addq %rdx, (%rsp)\n"
    "    ret\n"
);

void delay() {
    for (volatile int z = 0; z < 64; z++) {}    
}

uint64_t timer(uint8_t* ptr) {
    uint64_t clk;
    asm volatile (
        "rdtscp\n"
        "shl $32, %%rdx\n"
        "mov %%rdx, %%rsi\n"
        "or %%eax, %%esi\n"
        "mov (%1), %%al\n"
        "rdtscp\n"
        "shl $32, %%rdx\n"
        "or %%eax, %%edx\n"
        "sub %%rsi, %%rdx\n"
        "mov %%rdx, %0\n"
        : "=r" (clk)
        : "r" (ptr)
        : "rcx", "rdx", "rsi", "eax"
    );
    return clk;
}

void wm_and_gate(uint8_t *in1, uint8_t *in2, uint8_t *out) {
    asm volatile (
        "call mod_ret_addr\n"
        "mov $0, %%rcx\n"
        "mov (%[in1]), %%cl\n"    // dl = in2[in1[0]]
        "add %[in2], %%rcx\n"
        "mov $0, %%rdx\n"
        "mov (%%rcx), %%dl\n"
        "add %[out], %%rdx\n"     // dl = out[rdx]
        "mov (%%rdx), %%dl\n"
        : : [in1] "r"(in1), [in2] "r"(in2), [out] "r"(out) : "rax", "rcx", "rdx"
    );

    NOPs(NOP256);
}

bool __weird__and_gate(bool in1, bool in2, bool* out) {
    uint8_t regs[4 * 1024];

    regs[1 * 1024] = 0;
    regs[2 * 1024] = 0;

    // assign(&regs[1 * 1024], in1);
    if (!in1) _mm_clflush(&regs[1 * 1024]);
    // assign(&regs[2 * 1024], in2);
    if (!in2) _mm_clflush(&regs[2 * 1024]);
    _mm_clflush(&regs[3 * 1024]);
    
    delay();

    wm_and_gate(&regs[1 * 1024], &regs[2 * 1024], &regs[3 * 1024]); 
    
    delay();
    
    uint64_t clk = timer(&regs[3 * 1024]);
    out[0] = clk <= 200;

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
