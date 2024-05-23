# Flexo

A compiler for microarchitectural weird machines

## Install with Docker

```sh
docker build -t flexo .
docker run -i -t --rm \
  --mount type=bind,source="$(pwd)"/,target=/flexo \
  flexo \
  bash -c "cd /flexo && ./build.sh"
```

## Compile a circuit

To compile a circuit, you need to first compile the circuits (a C/C++ program) into LLVM IR. 

```sh
clang-17 -fno-discard-value-names -fno-inline-functions -O1 -S -emit-llvm [INPUT_CIRCUIT_SOURCE] -o [INPUT_LLVM_IR_FILE]
```

After that, run `compile.sh` to execute the Flexo compiler and compile the circuits into weird machines.
The output will be another LLVM IR file.

```sh
docker run -i -t --rm \
  --mount type=bind,source="$(pwd)"/,target=/flexo \
  flexo \
  bash -c "cd /flexo && ./compile.sh [INPUT_LLVM_IR_FILE] [OUTPUT_LLVM_IR_FILE]"
```

Finally, you can generate an executable file using the output LLVM IR file:

```sh
clang-17 [OUTPUT_LLVM_IR_FILE] -o [OUTPUT_EXECUTABLE_FILE] -lm -lstdc++
```
