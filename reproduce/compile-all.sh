#!/bin/bash

# configurations
min_div_rounds=1
max_div_rounds=50

# create a build dir if not exist
if ! test -d ./build; then
    mkdir build
fi

# compile the circuits into LLVM IR
make -C ../circuits

compile_circuit() {
    local circuit_name="$1"
    local dir="$2"
    local wr_offset=$3
    local verilog="${4:-}"

    if ! test -d ./build/$circuit_name; then
        mkdir ./build/$circuit_name
    fi
    for i in $(seq $min_div_rounds $max_div_rounds); do
        # config the Flexo compiler
        export RET_WM_DIV_ROUNDS=$i WR_OFFSET=$wr_offset WR_FAKE_OFFSET=256
        if [ -n "$verilog" ]; then
            export WM_CIRCUIT_FILE=$verilog
        else
            export WM_CIRCUIT_FILE=""
        fi
        
        # compile the circuit
        if (($i == $min_div_rounds)); then
            # output the size of the circuit once
            opt-17 -load-pass-plugin ../build/lib/libFlexo.so -passes="create-WMs" $dir/$circuit_name.ll -S -o ./build/$circuit_name/$circuit_name-$i.ll
        else
            opt-17 -load-pass-plugin ../build/lib/libFlexo.so -passes="create-WMs" $dir/$circuit_name.ll -S -o ./build/$circuit_name/$circuit_name-$i.ll > /dev/null
        fi

        # LLVM IR to executable
        clang-17 ./build/$circuit_name/$circuit_name-$i.ll -o ./build/$circuit_name/$circuit_name-$i.elf -lm -lstdc++
        echo -ne "Progress ($circuit_name): ${i}/${max_div_rounds}\r"
    done
    echo -ne '\n'
}

# compile single circuits (5.1 & 5.2)
compile_circuit ALU ../circuits/ALU  1088 ../circuits/ALU/ALU.v
compile_circuit sha1_round ../circuits/SHA1 576
compile_circuit aes_round ../circuits/AES 448
compile_circuit simon25 ../circuits/Simon 448
compile_circuit simon32 ../circuits/Simon 448

# compile composed circuits (5.3)
compile_circuit sha1_2blocks ../circuits/SHA1 576
compile_circuit aes_block ../circuits/AES 448
