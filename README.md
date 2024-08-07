# Flexo

A new design for microarchitectural weird machines, along with a compiler and a proof-of-concept packer application.

For more details, please refer to our paper:

> Ping-Lun Wang, Riccardo Paccagnella, Riad S. Wahby, Fraser Brown.
> "Bending microarchitectural weird machines towards practicality."
> USENIX Security, 2024.

## Table of contents

- [What are microarchitectural weird machines?](#what-are-microarchitectural-weird-machines)
- [Hardware requirements](#hardware-requirements)
- [Reproduce our results](#reproduce-our-results)
- [Install the Flexo compiler](#install-the-flexo-compiler)
- [Compile a weird machine](#compile-a-weird-machine)
- [Example: basic logic gates](#example-basic-logic-gates)
- [Create a new weird machine with C/C++](#create-a-new-weird-machine-with-cc)
- [UPFlexo: UPX packer with Flexo weird machines](#upflexo-upx-packer-with-flexo-weird-machines)
- [Run Flexo on an unsupported processor](#run-flexo-on-an-unsupported-processor)
- [Internals of the Flexo compiler](#internals-of-the-flexo-compiler)
- [Contacts](#contacts)

## What are microarchitectural weird machines?

Microarchitectural weird machines (µWMs) are code gadgets that perform computation purely through microarchitectural side effects.
They work similarly to a binary circuit: they use weird registers to store values and use weird gates to compute with them.

For example, here is a weird AND gate with two inputs and one output:

```c
Out[In1[0] + In2[0]]
```

`In1` and `In2` are the two input weird registers, and this AND gate outputs to the weird register `Out`.
Section 2.2 of our paper explains how a µWM works in details.
Note: the code snippets in our paper (Listing 1-5) are for illustrative purposes and can be different from our actual implementation, which we show in Listing 7 in the appendix.

These µWMs can prevent both static and dynamic analysis because they convert computations into memory operations (like the AND gate example above), and debuggers and emulators may not preserve the microarchitecture behavior of a processor, e.g., single-stepping stops transient execution.

Therefore, they are a great candidate for program obfuscation and potentially many other types of attacks.

## Hardware requirements

The following list is the AWS EC2 instances that we used to run our Flexo weird machines.

| Microarchitecture | Instance type | Processor         |
| ----------------- | ------------- | ----------------- |
| Zen 1             | t3a.xlarge    | AMD EPYC 7571     |
| Zen 2             | c5a.xlarge    | AMD EPYC 7R32     |
| Zen 3             | c6a.xlarge    | AMD EPYC 7R13     |
| Zen 4             | m7a.xlarge    | AMD EPYC 9R14     |
| Skylake           | c5n.xlarge    | Intel Xeon 8124M  |
| Cascade Lake      | m5n.xlarge    | Intel Xeon 8259CL |
| Icelake           | m6in.xlarge   | Intel Xeon 8375C  |
| Sapphire Rapids   | m7i.xlarge    | Intel Xeon 8488C  |

> [!WARNING]
> When using a processor not included in this list, the weird machines may fail to generate correct results.
> While a processor with similar microarchitecture may be able to run our weird machines, we cannot guarantee the accuracy and performance when using other processors.
> For instructions about running our Flexo weird machine on an unsupported processor, please refer to "[Run Flexo on an unsupported processor](#run-flexo-on-an-unsupported-processor)".

## Reproduce our results

Follow the instructions in the [README](reproduce/README.md) file under `reproduce/` to run the experiments in our paper.

## Install the Flexo compiler

We suggest installing the Flexo compiler using a Docker or Podman container.
The script below creates a container using the provided [Docker file](./Dockerfile) and runs the [build script](./build.sh) to build the compiler.
For installation using Podman, simply replace `docker` with `podman` in these commands.

```sh
docker build -t flexo .
docker run -i -t --rm \
  --mount type=bind,source="$(pwd)"/,target=/flexo \
  flexo \
  bash -c "cd /flexo && ./build.sh"
```

The compiler will be stored inside the `build/` folder when the installation process is complete.

## Compile a weird machine

### 1. C/C++ to LLVM IR

The Flexo compiler takes a LLVM IR file as input, so the first step of compiling a weird machine is to compile a C/C++ program implementing the weird machine into LLVM IR.
The following command uses `clang` (version 17) to compile a C/C++ program into LLVM IR.
Replace `[INPUT_WM_SOURCE]` with the filename of the input C/C++ program and `[LLVM_IR_FILE]` with the filename of the output LLVM IR.

```sh
clang-17 -fno-discard-value-names -fno-inline-functions -O1 -S -emit-llvm [INPUT_WM_SOURCE] -o [LLVM_IR_FILE]
```

For more information about how to write a C/C++ program that implements a weird machine, please refer to #TODO.
It is also possible to implement a weird machine using structural Verilog.
For more information about using Verilog to create a weird machine, please refer to #TODO.

### 2. Use the Flexo compiler to generate a weird machine

After that, run `compile.sh` to execute the Flexo compiler and generate the weird machine, which is also in the form of LLVM IR.
The following command performs this step.
Remember to replace `[INPUT_LLVM_IR_FILE]` with the filename of the LLVM IR file generated in the previous step and `[OUTPUT_LLVM_IR_FILE]` with the filename of the output LLVM IR file that contains the weird machine.

```sh
docker run -i -t --rm \
  --mount type=bind,source="$(pwd)"/,target=/flexo \
  flexo \
  bash -c "cd /flexo && ./compile.sh [INPUT_LLVM_IR_FILE] [OUTPUT_LLVM_IR_FILE]"
```

The Flexo compiler provides several compile options to adjust the construction of a weird machine.
For the details of these compiler options, please refer to #TODO

### 3. Generate an executable file

Finally, you can generate an executable file from the output LLVM IR file using the following command.
Remember to replace `[OUTPUT_LLVM_IR_FILE]` with the filename of the LLVM IR file generated from the previous step and `[OUTPUT_EXECUTABLE_FILE]` with the filename of the executable file to be generated.

```sh
clang-17 [OUTPUT_LLVM_IR_FILE] -o [OUTPUT_EXECUTABLE_FILE] -lm -lstdc++
```

## Example: basic logic gates

We implemented several weird machines: basic logic gates, adders, multipliers, a 4-bit ALU, the SHA-1 hash function, the AES block cipher, and the Simon block cipher.
These examples can be found in the [`circuits/`](./circuits/) folder.

The [readme file](./circuits/gates/README.md) under [`circuits/gates`](./circuits/gates/) provides instructions regarding how to build a simple weird machine that computes basic logic gates (`AND`, `OR`, `NOT`, `NAND`, and `MUX`).

## Create a new weird machine with C/C++

#TODO

## UPFlexo: UPX packer with Flexo weird machines

#TODO

## Run Flexo on an unsupported processor

#TODO

## Internals of the Flexo compiler

#TODO

## Contacts

For any question, contact the first author: Ping-Lun Wang (pinglunw \[at\] andrew \[dot\] cmu \[dot\] edu).
