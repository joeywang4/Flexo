# SHA-1

There are two weird machines in this example:

1. [SHA-1 round](./sha1_round.c): the first round of the SHA-1 hash function. It does not compute the complete SHA-1 hash function.
2. [SHA-1 2 blocks](./sha1_2blocks.c): the complete SHA-1 hash function with two input blocks.

## Compile the weird machine

Follow the steps in "[Compile a weird machine](../../README.md#compile-a-weird-machine)" from the [readme](../../README.md) of Flexo to compile these weird machines.
There are two weird machines inside this folder, [sha1_round.c](./sha1_round.c) and [sha1_2blocks.c](./sha1_2blocks.c).
Use the [Makefile](./Makefile) to compile them into LLVM IR, and then move on to the second step of the compilation process.
