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
- [Configure the Flexo compiler](#configure-the-flexo-compiler)
- [Create a new weird machine with C/C++](#create-a-new-weird-machine-with-cc)
- [UPFlexo: UPX packer with Flexo weird machines](#upflexo-upx-packer-with-flexo-weird-machines)
- [Run Flexo on an unsupported processor](#run-flexo-on-an-unsupported-processor)
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

## Configure the Flexo compiler

The Flexo compiler can be configured by setting environment variables.
Line 15 of the [compilation script](./compile.sh#L15) provides an example of configuring the Flexo compiler using environment variables.
The following is the list of environment variables that are available to configure the compiler.

### General configuration

- `WM_KEYWORD`: The compiler only converts a function to WM when it contains a "keyword" in its name. The default value of this keyword is `__weird__`.
- `WM_VERBOSE`: Show verbose output, default to `false`.
- `TMP_PATH`: A folder to store temporary files, default to `/tmp`.
- `WM_CIRCUIT_FILE`: The path to a Verilog circuit file when the WM is implemented using Verilog. No default value. See [the ALU example](./circuits/ALU/README.md) for this configuration.

### Weird machine configuration

- `WM_DELAY`: The iterations of the delay loop between gate executions. Default to `256`.
- `WM_USE_FENCE`: Use memory fences instead of using a delay loop. Default to `true`.
- `RET_WM_DIV_ROUNDS`: The number of `div` instructions used when calculating the modified return address. Default to `4`.
- `RET_WM_DIV_SIZE`: The register size of the `div` instructions when calculating the modified return address. Possible values: `16` (default), `32`, or `64`.
- `RET_WM_JMP_SIZE`: The jump size of the modified return address (should be larger than the gate size). Default to `512`.
- `DUAL_WM_MAX_INPUT`: The maximum input size of a weird gate. Default to `4`.
- `WM_MAX_FANOUT`: The max number of fan-outs of an assign gate. Default to `3`.

### Weird register configuration

- `WR_TYPE`: The weird register type, which can be `Baseline`, `NoBranch`, or `Dual` (default).
- `WR_MAPPING`: The mapping type of weird registers, which can be `Baseline` or `Shuffle` (default).
- `WR_OFFSET`: The memory offset (in bytes) between each weird registers. Default to `960`.
- `WR_FAKE_OFFSET`: The memory offset (in bytes) between a real weird register and its fake location. Default to `512`.
- `WR_HIT_THRESHOLD`: The maximum access latency (in cycles) of a cache hit. Default to `180`.
- `WR_SYSCALL_RAND`: Use Linux syscall to generate random numbers instead of using standard library calls. Default to `false`.
- `WR_USE_MMAP`: Call the `mmap` syscall to allocate memory for WR instead of using the stack memory. May be slower if enabled, but this supports circuits with more wires. Default to `false`.

## Create a new weird machine with C/C++

The Flexo compiler converts C or C++ functions into weird machines.
These functions must comply with the following requirements:

### Function name

The functions that should be converted to a weird machine must contain the string `__weird__` in its name.
This string can be customized by specifying `WM_KEYWORD` in compiler configuration.

### Function type

The functions that should be converted to a weird machine must follow a special format.
Here is an example:

```cpp
bool __weird__fn(unsigned char* in1, unsigned char* out1, bool in2, bool* out2, bool& out3);
```

There are no rules for the names of the arguments or the order of the arguments, so they can be named or placed arbitrarily.

The inputs and outputs of a weird machine are all contained in the function argument.
There is no need to specify an argument as input or output as the compiler will detect this automatically by checking if the value of an argument is read or over-written.
The input and output variables must have the following types:

#### Input variable types

Inputs can be integer type with arbitrary length, e.g., `int`, `unsigned char`, `long`, are all valid.
They can be passed by value, by pointer, or by reference.

#### Output variable types

Outputs also can integer type with arbitrary length, but they must be passed by pointer or by reference.

#### Return value

The return value of the function is **NOT** the output of the weird machine, and it instead contains the error detection result.
The return value is `true` when an error is detected and `false` if undetected.
When the output of the weird machine has more than one bit or when there are multiple outputs, then the return value is `true` when **any of the output bit** detects an error.

If the error detection value is not needed, the return type of the function can be set to `void`.

#### Bit-wise error detection

Flexo weird machines can perform error detection for each output bit.
The compiler provides these error detection results via some special output variables, which have the name of the output variable with the `error_` prefix.

For example:

```cpp
void __weird_fn2(unsigned char* in, unsigned char* out, unsigned char* error_out);
```

This function has an output `out`, and the bit-wise error detection results of this output are provided in `error_out`.
`error_out` must have the same length as `out`, and the bits in `error_out` are set when the corresponding bits in `out` detect an error.
For example, if the n-th bit of `out` detects an error, then the n-th bit of `error_out` is set to `1`; otherwise, the n-th bit of `error_out` is set to `0`.

### Function body

The Flexo compiler parses the LLVM IR instructions inside the function body and converts it to a Verilog circuit.
The conversion is simple: it translates a LLVM IR instruction to a corresponding Verilog operator.
For example, the `Add` instruction is converted to `+`.

As some functionality of LLVM IR is not supported by a Verilog circuit, some LLVM IR instructions are not allowed in the function body, and any unsupported LLVM IR instruction will lead to compile errors.
For example, the function body should not contain any branch or function call, and memory operations are not allowed except for the inputs and outputs.
To see what LLVM IR instructions are supported, please refer to [`lib/Circuits/IRParser.cpp`](./lib/Circuits/IRParser.cpp).

## UPFlexo: UPX packer with Flexo weird machines

As a proof-of-concept application, we obfuscated [the UPX packer](https://upx.github.io/) with Flexo.
We encrypt the packed binary using AES or Simon encryption, and at runtime, a Flexo weird machine decrypts the packed binary.
Please refer to the [readme](./UPFlexo/README.md) of UPFlexo for more information about how to install and use this packer.

## Run Flexo on an unsupported processor

Microarchitectural weird machines are very sensitive to the nuances of a microarchitecture.
When running Flexo on an unsupported processor or when creating a new weird machine, some compiler configuration may need to be adjusted so that the weird machine can execute correctly.
Here are some compiler configurations that should be adjusted when tuning a weird machine:

### Adjust the transient window by setting `RET_WM_DIV_ROUNDS` or `RET_WM_DIV_SIZE`

The length of the transient window is controlled by the `RET_WM_DIV_ROUNDS` environment variable.
(For the definition of a "transient window", please refer to section 2.1 of our paper.)
We suggest trying every integers between 1 and 50 to see which number gives the best result.

For some rare cases, `RET_WM_DIV_SIZE` may also need to be adjusted.

### Adjust the spacing between weird registers by setting `WR_OFFSET`

The spacing between weird registers can impact the accuracy of a weird machine significantly.
We discuss this in section 3.3 of our paper.
We suggest setting `WR_OFFSET` to $2^n \pm L$, where $L$ is the L1 cache line size (usually `64`) and $n \in [8, 12]$.
I.e., set `WR_OFFSET` to 192, 320, 448, 576, 960, 1088, ...

### Reduce the size of the gate by adjusting `DUAL_WM_MAX_INPUT`

Some processors may have smaller transient windows, and thus they can only support small weird gates.
In this case, reduce `DUAL_WM_MAX_INPUT` to `3` or `2` to reduce the size of weird gates.

## Contacts

For any question, contact the first author: Ping-Lun Wang (pinglunw \[at\] andrew \[dot\] cmu \[dot\] edu).
