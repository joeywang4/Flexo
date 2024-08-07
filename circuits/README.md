# Examples of Flexo weird machines

We implemented several Flexo weird machines for the experiments in our paper and to show how to implement a microarchitectural weird machine using C/C++. This includes:

- [`gates/`](./gates/): basic logic gates. This is the simplest weird machine and a great starting point to learn how to create µWMs.
- [`arithmetic`](./arithmetic/): adders and multipliers.
- [`ALU`](./ALU/): a small 4-bit ALU. This examples shows how to implement a µWM using Verilog and use a C/C++ program to execute the weird machine.
- [`SHA1`](./SHA1/): the SHA-1 hash function.
- [`AES`](./AES/): the AES block cipher.
- [`Simon`](./Simon/): the Simon block cipher.

## Build and run the examples

We provide a [Makefile](./Makefile) to compile every examples here.
However, this [Makefile](./Makefile) only compiles the examples into LLVM IR.
After that, it is still required to run steps 2 and 3 of ["Compile a weird machine"](../README.md#compile-a-weird-machine) to build the weird machines into executable files.
For more information regarding how to build and run each example, please refer to readme file under their own folder.
