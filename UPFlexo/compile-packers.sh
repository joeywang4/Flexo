#!/bin/bash

# configurations
min_div_rounds=1
max_div_rounds=50

# create a build dir if not exist
if ! test -d build; then
    mkdir build
fi
if ! test -d build/upx; then
    mkdir build/upx
fi

# create podman image to build runtime stubs
./misc/podman/rebuild-stubs/10-create-image.sh

compile_upx() {
    local circuit_name="$1"
    local is_simon="$2"
    local is_simon25="$3"

    # create a dir for the circuit if not exist
    if ! test -d ./build/$circuit_name; then
        mkdir ./build/$circuit_name
    fi

    # configure upx's encryption algorithm
    cmake -DSIMON=$is_simon -DSIMON25=$is_simon25 -G Ninja -S . -B build/upx

    # generate circuits with different transient window sizes
    # (by adjusting the number of div instructions)
    for div_round in $(seq $min_div_rounds $max_div_rounds); do
        # copy compiled circuit to UPX's runtime stub folder
        cp WM/build/$circuit_name/$circuit_name-$div_round.S src/stub/src/amd64-linux.elf-WM.S

        # compile runtime stub
        ./misc/podman/rebuild-stubs/20-image-run-shell.sh make -C src/stub all > /dev/null

        # compile UPX
        ninja -C build/upx > /dev/null

        # move the executable file to other dir
        mv build/upx/upx build/$circuit_name/upx-$circuit_name-$div_round.elf

        echo -ne "Progress ($circuit_name): ${div_round}/${max_div_rounds}\r"
    done
    echo -ne '\n'
}

compile_upx simon25_CTR 1 1
compile_upx simon32_CTR 1 0
compile_upx AES_CTR 0 0

# cp WM/build/simon25_CTR/simon25_CTR-10.S src/stub/src/amd64-linux.elf-WM.S
# ./misc/podman/rebuild-stubs/20-image-run-shell.sh make -C src/stub all
# cmake -DSIMON=1 -DSIMON25=1 -G Ninja -S . -B build/upx
# ninja -C build/upx
