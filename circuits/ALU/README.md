# ALU

This folder contains the implementation of a 4-bit ALU.
We implement this weird machine using Verilog, and the circuit file is [`ALU.v`](./ALU.v).
The C++ program ([`ALU.cpp`](./ALU.cpp)) triggers this weird machine and measures its accuracy and runtime.
This demonstrates that the Flexo compiler also allows developers to create a µWM using a Verilog circuit if they prefer to control the low-level details of a weird machine circuit.

## Compile the weird machine

Since this weird machine is implemented using Verilog, the compilation process is slightly different from the steps outlined in "[Compile a weird machine](../../README.md#compile-a-weird-machine)".
Specifically, in step 2, it is required to add an environment variable to configure the Flexo compiler so that it will read the Verilog file.
To do this, modify line 15 of [`compile.sh`](../../compile.sh) to the following line:

```sh
export RET_WM_DIV_ROUNDS=5 WR_OFFSET=576 WM_CIRCUIT_FILE=./circuits/ALU/ALU.v
```

The `WM_CIRCUIT_FILE` variable is set to the path of the Verilog file, and the Flexo compiler will then use it as the weird machine circuit during the compilation process.

After that, follow the original step 3 to create an executable file of this weird machine.
