# AES

There are two weird machines in this example:

1. [AES round](./aes_round.c): one round of the AES encryption. It does not compute the complete AES block.
2. [AES block](./aes_block.c): the complete AES encryption with one block.

## Compile the weird machine

Follow the steps in "[Compile a weird machine](../../README.md#compile-a-weird-machine)" from the [readme](../../README.md) of Flexo to compile these weird machines.
There are two weird machines inside this folder, [aes_round.c](./aes_round.c) and [aes_block.c](./aes_block.c).
Use the [Makefile](./Makefile) to compile them into LLVM IR, and then move on to the second step of the compilation process.
