#!/bin/bash

# create a build dir if not exist
if ! test -d ./build; then
    mkdir build
fi

# build the compiler
cmake -DLT_LLVM_INSTALL_DIR=/usr/lib/llvm-17 -G Ninja -B build -S .
ninja -C build
