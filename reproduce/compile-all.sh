#!/bin/bash

# configurations
min_div_rounds=1
max_div_rounds=50

# constant values
rcx_const=9223372035050486425
rax_const=(1528108405744583850 15425805989619775056 9590474711649734392 13261519319564870468 12405458456678603866 1500495089402237213 11447395069359229456 14381340116075026752 9052090916964150814 2550717117220168909 3947894985339143883 18075015426062516027 10284092648529431303 1699415875179475693 11040943603007220635 4666665506667876849 9776573533221304541 10286860886423803780 296762250051013680 16805791835971901241 277159706224019526 1489808779278174161 294910195971490501 17007366770701626811 15149847456915230433 3012477595696286550 7795531398997871635 15318261096864505921 17573673945437251516 5112792816473927524 5306590219030172128 14844099282241860038 9261135744010105521 16337051782604493586 8930788443912704999 6193619819956692978 17100971486527309623 159777734954248724 11394821881762108343 13942107503458057377 10859601353695111352 12555710877131426691 10566797404024617390 14518152341679405795 3777229509924082447 16992332698284675937 7656239271347670673 18162272589806296695 6396140677861283593 987112254011377951)
rdx_const=(9223372034627020982 764054202722826536 7712902993301078284 4795237354886815926 6630759658485316435 6202729227125915067 750247544554354096 5723697533559936734 7190670056630864095 4526045457596683846 1275358558360596968 1973947492283425476 9037507711263327678 5142046323258821110 849707937423516750 5520471800423687636 2333332752877488495 4888286765654398540 5143430442205736585 148381124996480309 8402895916342163963 138579853084900572 744904389493367806 147455097956899870 8503683383687310557 7574923726975797677 1506238797553490662 3897765698736449251 7659130546933962760 8786836970999731996 2556396407736877798 2653295108996044635 7422049639669017929 4630567871099214373 8168525889704307939 4465394221082825621 3096809909372544053 8550485741590996419 79888867461496386 5697410939766518399 6971053750365341067 5429800675785370211 6277855437337630181 5283398700978762596 7259076169419671914 1888614754592587672 8496166347480305614 3828119634924973034 9081136293126683326 3198070338305030650)

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
    export RET_WM_DIV_ROUNDS=$min_div_rounds WR_OFFSET=$wr_offset WR_FAKE_OFFSET=256 WR_HIT_THRESHOLD=180
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
            sed -i "${lineNum}a module asm \"    div  %rcx\"" $newfile
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
