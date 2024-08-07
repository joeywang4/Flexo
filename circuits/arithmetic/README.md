# Arithmetic circuits: adders and multipliers

There are two weird machines in this example:

1. [Adders](./adder.cpp): input size range is 2-bit to 64-bit
2. [Multipliers](./mul.cpp): input size range is 2-bit to 16-bit

## Compile the weird machine

Follow the steps in "[Compile a weird machine](../../README.md#compile-a-weird-machine)" from the [readme](../../README.md) of Flexo to compile these weird machines.
There are two weird machines inside this folder, [adder.cpp](./adder.cpp) and [mul.cpp](./mul.cpp).
Use the [Makefile](./Makefile) to compile them into LLVM IR, and then move on to the second step of the compilation process.
