#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# configurations
min_div_rounds=1
max_div_rounds=50

# create a build dir if not exist
if ! test -d $SCRIPT_DIR/build; then
    mkdir $SCRIPT_DIR/build
fi
if ! test -d $SCRIPT_DIR/build/upx; then
    mkdir $SCRIPT_DIR/build/upx
fi

# create podman image to build runtime stubs
$SCRIPT_DIR/misc/podman/rebuild-stubs/10-create-image.sh

compile_upx() {
    local circuit_name="$1"
    local is_simon="$2"
    local is_simon25="$3"

    # create a dir for the circuit if not exist
    if ! test -d $SCRIPT_DIR/build/$circuit_name; then
        mkdir $SCRIPT_DIR/build/$circuit_name
    fi

    # configure upx's encryption algorithm
    podman run -i -t --rm \
        --mount type=bind,source="$SCRIPT_DIR/.."/,target=/flexo \
        flexo \
        bash -c "cd /flexo/UPFlexo && cmake -DSIMON=$is_simon -DSIMON25=$is_simon25 -G Ninja -S . -B build/upx"

    # generate circuits with different transient window sizes
    # (by adjusting the number of div instructions)
    for div_round in $(seq $min_div_rounds $max_div_rounds); do
        # copy compiled circuit to UPX's runtime stub folder
        cp $SCRIPT_DIR/WM/build/$circuit_name/$circuit_name-$div_round.S $SCRIPT_DIR/src/stub/src/amd64-linux.elf-WM.S

        # compile runtime stub
        $SCRIPT_DIR/misc/podman/rebuild-stubs/20-image-run-shell.sh make -C src/stub all > /dev/null

        # compile UPX
        podman run -i -t --rm \
            --mount type=bind,source="$SCRIPT_DIR/.."/,target=/flexo \
            flexo \
            bash -c "cd /flexo/UPFlexo && ninja -C build/upx" > /dev/null
        

        # move the executable file to other dir
        mv $SCRIPT_DIR/build/upx/upx $SCRIPT_DIR/build/$circuit_name/upx-$circuit_name-$div_round.elf

        echo -ne "Progress ($circuit_name): ${div_round}/${max_div_rounds}\r"
    done
    echo -ne '\n'
}

compile_upx simon25_CTR 1 1
compile_upx simon32_CTR 1 0
compile_upx AES_CTR 0 0
