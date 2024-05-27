f_decrypt:
    push %rax
    push %rbx
    push %rcx
    // preserve register used by mmap
    push %arg6 // r9
    push %arg5 // r8
    push %sys4 // r10
    push %arg3 // rdx
    push %arg2 // rsi
    push %arg1 // rdi

// Reserve space for unfolded stub.
    subq %arg6,%arg6  // 0 offset
    orl $-1,%arg5l  // fd
    push $MAP_PRIVATE|MAP_ANONYMOUS
    pop %sys4
    // push %rdx  // len (already in %arg2)
    // pop %arg2
    push $PROT_READ|PROT_WRITE
    pop %arg3
    subl %arg1l,%arg1l  // 0; kernel chooses addr
    push $__NR_mmap
    pop %rax
    syscall
    movq %rax,%rcx  // ADRU (output)
    pop %arg1
    push %rax // save output

    // mov %arg1, %rdi // input (rdi -> rdi)
    mov %arg2, %rdx // src length (rsi -> rdx)
    mov %rcx, %rsi // output (rcx -> rsi)
    call f_weird_decrypt // defined in amd64-linux.elf-WM.S
    mov %rax, %arg3 // new src length (rax -> rdx)

    // restore registers
    pop %rax
    pop %arg2
    mov %arg3, %arg2 // new src length (rdx -> rsi)
    pop %arg3
    pop %sys4
    pop %arg5
    pop %arg6
    pop %rcx
    pop %rbx
    mov %rax, %arg1 // modify src to the new addr
    pop %rax
    ret

#include "amd64-linux.elf-WM.S"
