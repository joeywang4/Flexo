#!/bin/bash

PROJECT_ROOT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )/../../

# remove every build & results folder, and remove circuits' LLVM IR
rm -rf $PROJECT_ROOT/build
make -C $PROJECT_ROOT/circuits clean
rm -rf $PROJECT_ROOT/reproduce/build
rm -rf $PROJECT_ROOT/reproduce/results
rm -rf $PROJECT_ROOT/UPFlexo/build
rm -rf $PROJECT_ROOT/UPFlexo/WM/build
