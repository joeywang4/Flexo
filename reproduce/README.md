# Reproducing the results in our paper

After installing Flexo (see the [README](../README.md) file in the root directory for instructions), it is possible to reproduce our experiment results by following the instructions below.

## Circuits (section 5)

Follow these steps to reproduce the results in section 5 of our paper.

### 1. Compile the circuits

We provide a script to compile every circuits: `compile-all.sh`.
To execute this script using the installed Flexo docker image, run the following command:

```sh
docker run -i -t --rm \
  --mount type=bind,source="$(pwd)/.."/,target=/flexo \
  flexo \
  bash -c "cd /flexo/reproduce && ./compile-all.sh"
```

It takes around 10 mintes to compile every circuits.
The circuits are roughly 352 MB in size.

### 2. Run the circuits

We provide a Python script ([run_WM.py](./run_WM.py)) to run and collect the results of each circuit.
To run this script, please make sure Python 3 is installed.

```sh
python3 run_WM.py
```

__Command line options__

- `-f` or `--fast`: enable this flag to reduce the experiment time. Without this flag, it may take as long as 7 ~ 8 hours to finish the experiment. Enabling the "fast" flag reduces the experiment time to less than 3 ~ 4 hours. However, using this flag may generate less stable results, which can be very different from the results in our paper.
- `--simon25`: reduce the number of rounds in the Simon encryption circuits from 32 rounds to 25 rounds. We enable this flag on the Zen 3 machine.
- `-r` or `--report`: report previous experiment results (stored in the `results/` folder) without running the experiments.

__Customizing the experiment settings__

This Python script reads in a config file ([config.csv](./config.csv), or [config-fast.csv](./config-fast.csv) when the "fast" flag is enabled) to determine the number of iterations or trials when running each circuit, as well as the timeout for the circuits. It is possible to adjust the values inside these config files to increase or decrease the experiment time for a certain circuit.

The config file has the following fields:
1. `WM`: the name of the circuit
2. `Test window iterations`: before measuring the accuracy and runtime of a circuit, the script first determines the best transient window size of a circuit in the current execution environment. It runs a circuit with different window sizes and picks the one with the highest accuracy. When running the circuits, the script uses the value inside this field to control the number of iterations to run a circuit. For example, when this field is set to `100`, then, for each transient window size, the script will execute the circuit 100 times to calculate the accuracy.
3. `Measure trials`: after the best transient window size is determined, the script will run the circuit multiple times to collect the accuracy and runtime multiple times. It will then output the median of the collected results. This field controls the number of times a circuit is executed during this step. During this step, the error correction is turned off.
4. `Measure timeout (minutes)`: configures the timeout (in minutes) when measuring the accuracy and runtime of a circuit. When a timeout expires, the script saves the current results and stops executing this circuit.
5. `Measure trials with EC`: after measuring the accuracy and runtime of a circuit without error correction, the script continues to measure the accuracy and runtime for the same circuit with error correction enabled. Similar to `Measure trials`, this field controls the number of times a circuit is executed during this measurement.
6. `Measure timeout with EC (minutes)`: configures the timeout (in minutes) when measuring the accuracy and runtime of a circuit (with error correction). When a timeout expires, the script saves the current results and stops executing this circuit.

## UPFlexo packer (section 6)

Follow these steps to reproduce the results in section 6 of our paper.

### 1. Compile the packers

Please refer to the [readme file](/UPFlexo/README.md) under `UPFlexo/` for instructions about how to compile the packers.

### 2. Pack the example program (`ls`)

Run the `pack-all.sh` script under this directory to pack the example program, which is the `ls` binary from Ubuntu 23.04.
It takes around 20 seconds to pack the example program, and the packed programs are roughly 233 MB in size.

### 3. Run the packed programs

We provide a Python script ([run_packed.py](./run_packed.py)) to run and collect the results of each packer.
To run this script, please make sure Python 3 is installed.

```sh
python3 run_packed.py
```

__Command line options__

- `-f` or `--fast`: reduce the experiment time. Without this flag, it may take as long as 4 hours to finish the experiment. Enabling the "fast" flag reduces the experiment time to less than 1 hour. However, using this flag may generate less stable results, which can be very different from the results in our paper.
- `--simon25`: reduce the number of rounds in Simon decryption from 32 rounds to 25 rounds. We enable this flag on the Zen 3 machine.
- `-r` or `--report`: report previous experiment results (stored in the `results/` folder) without running the experiments.
