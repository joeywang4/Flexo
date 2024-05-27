#! /usr/bin/env bash
if [[ $# == 0 ]]; then
    compiler_path="/WM-compiler/build/"
else
    compiler_path=$1
fi

TARGET=Simon32_CTR

# compile to LLVM IR
make -C WM

# Config and run the pass
export WM_DUAL=true WR_TYPE=Dual DUAL_WM_MAX_INPUT=4 WR_OFFSET=448 WR_FAKE_OFFSET=256 WR_SYSCALL_RAND=true RET_WM_DIV_ROUNDS=11 RET_WM_DIV_SIZE=16 # WM_KEYWORD="____"
opt-17 -load-pass-plugin $compiler_path/lib/libTranslator.so -passes="translate-to-WM" WM/build/${TARGET}.ll -S -o WM/build/compiled.ll

# LLVM IR -> assembly
clang-17 -S WM/build/compiled.ll -o WM/build/amd64-linux.elf-WM.S

# remove head and tail of the assembly file
sed -i -n '/mod_ret_addr/,$p' WM/build/amd64-linux.elf-WM.S
sed -i '/.ident/,$d' WM/build/amd64-linux.elf-WM.S
# UPX stub compiler does not support rdtscp instruction
sed -i 's/rdtscp/.byte	0x0F, 0x01, 0xF9/g' WM/build/amd64-linux.elf-WM.S
