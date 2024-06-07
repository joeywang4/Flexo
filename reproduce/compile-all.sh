#!/bin/bash

# configurations
min_div_rounds=1
max_div_rounds=50

# constant values
rcx_const=15001
rax_const=(50346 16386 57910 31980 21715 40115 18533 19003 52181 9546 15330 1691 7768 5147 10517 21543 10934 54793 65319 34124 7379 9886 59143 45573 48094 48237 31269 36158 37379 4639 64499 42592 26891 27560 32627 21087 56819 50965 61530 14171 60007 31290 26193 40035 64662 5489 42074 41650 46592 60221)
rdx_const=(12904 11524 3750 13255 7320 4970 9182 4242 4349 11944 2185 3509 387 1778 1178 2407 4931 2502 12541 14951 7811 1689 2262 13537 10431 11008 11041 7157 8276 8556 1061 14763 9749 6155 6308 7468 4826 13005 11665 14084 3243 13735 7162 5995 9163 14801 1256 9630 9533 10664)

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

    # create a dir for the circuit if not exist
    if ! test -d ./build/$circuit_name; then
        mkdir ./build/$circuit_name
    fi

    # config the Flexo compiler and compile the circuit with $min_div_rounds
    export RET_WM_DIV_ROUNDS=$min_div_rounds WR_OFFSET=$wr_offset
    if [ -n "$verilog" ]; then
        export WM_CIRCUIT_FILE=$verilog
    else
        export WM_CIRCUIT_FILE=""
    fi

    # run the Flexo compiler and create an executable
    outfile=./build/$circuit_name/$circuit_name-$min_div_rounds.ll
    opt-17 -load-pass-plugin ../build/lib/libFlexo.so -passes="create-WMs" $dir/$circuit_name.ll -S -o $outfile    
    clang-17 $outfile -o ./build/$circuit_name/$circuit_name-$min_div_rounds.elf -lm -lstdc++

    # generate circuits with different transient window sizes
    # (by adjusting the number of div instructions)
    for div_round in $(seq $(expr $min_div_rounds + 1) $max_div_rounds); do
        let "i = $div_round - 1"
        
        # copy outfile
        newfile=./build/$circuit_name/$circuit_name-$div_round.ll
        cp $outfile $newfile

        # find the line where division starts
        lineNum=$(awk "/${rcx_const}/{ print NR; exit }" $newfile)
        let lineNum++
        
        # modify the constants
        sed -i "${lineNum}s/.*/module asm \"    mov \$${rax_const[$i]}, %rax\"/" $newfile
        let lineNum++
        sed -i "${lineNum}s/.*/module asm \"    mov \$${rdx_const[$i]}, %rdx\"/" $newfile
        let lineNum++
        for j in $(seq 1 $i); do
            sed -i "${lineNum}a module asm \"    div  %cx\"" $newfile
        done
        
        # LLVM IR to executable
        clang-17 $newfile -o ./build/$circuit_name/$circuit_name-$div_round.elf -lm -lstdc++
        # delete LLVM IR to save space
        rm $newfile
        echo -ne "Progress ($circuit_name): ${div_round}/${max_div_rounds}\r"
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
