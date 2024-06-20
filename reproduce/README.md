# Reproducing the results in our paper

Follow the instructions below to reproduce our experiment results.

## Building the Flexo packer, circuits, packers, and packed programs

First, ensure dependencies are installed, and all submodules are pulled:

```bash
sudo apt install podman python3
git submodule update --init --recursive
```

Then, run `build-all.sh` to compile everything.

```bash
./scripts/build-all.sh
```

This script takes roughly 1 hour to compile everything and consumes around 10 GB of disk space.

## Run the circuits and the packed programs

### Circuits (section 5)

We provide a Python script ([run_WM.py](./scripts/run_WM.py)) to run and collect the results of each circuit.

```sh
python3 ./scripts/run_WM.py
```

__Command line options__

- `-f` or `--fast`: enable this flag to reduce the experiment time. Without this flag, it may take as long as 7 ~ 8 hours to finish the experiment. Enabling the "fast" flag reduces the experiment time to less than 3 ~ 4 hours. However, using this flag may generate less stable results, which can be very different from the results in our paper.
- `--simon25`: reduce the number of rounds in the Simon encryption circuits from 32 rounds to 25 rounds. We enable this flag on the Zen 3 machine.
- `-r` or `--report`: report previous experiment results (stored in the `results/` folder) without running the experiments.

__Customizing the experiment settings__

This Python script reads in a config file ([config.csv](./scripts/config.csv), or [config-fast.csv](./scripts/config-fast.csv) when the "fast" flag is enabled) to determine the number of iterations or trials when running each circuit, as well as the timeout for the circuits. It is possible to adjust the values inside these config files to increase or decrease the experiment time for a certain circuit.

The config file has the following fields:
1. `WM`: the name of the circuit
2. `Test window iterations`: before measuring the accuracy and runtime of a circuit, the script first determines the best transient window size of a circuit in the current execution environment. It runs a circuit with different window sizes and picks the one with the highest accuracy. When running the circuits, the script uses the value inside this field to control the number of iterations to run a circuit. For example, when this field is set to `100`, then, for each transient window size, the script will execute the circuit 100 times to calculate the accuracy.
3. `Measure trials`: after the best transient window size is determined, the script will run the circuit multiple times to collect the accuracy and runtime multiple times. It will then output the median of the collected results. This field controls the number of times a circuit is executed during this step. During this step, the error correction is turned off.
4. `Measure timeout (minutes)`: configures the timeout (in minutes) when measuring the accuracy and runtime of a circuit. When a timeout expires, the script saves the current results and stops executing this circuit.
5. `Measure trials with EC`: after measuring the accuracy and runtime of a circuit without error correction, the script continues to measure the accuracy and runtime for the same circuit with error correction enabled. Similar to `Measure trials`, this field controls the number of times a circuit is executed during this measurement.
6. `Measure timeout with EC (minutes)`: configures the timeout (in minutes) when measuring the accuracy and runtime of a circuit (with error correction). When a timeout expires, the script saves the current results and stops executing this circuit.

### Packed programs (section 6)

We provide a Python script ([run_packed.py](./scripts/run_packed.py)) to run and collect the results of each packer.

```sh
python3 ./scripts/run_packed.py
```

__Command line options__

- `--aes-timeout`: set the timeout (in minutes) when running the AES circuit. (Default to 120 minutes)
- `--simon-timeout`: set the timeout (in minutes) when running the Simon circuit. (Default to 120 minutes)
- `--simon25`: reduce the number of rounds in Simon decryption from 32 rounds to 25 rounds. We enable this flag on the Zen 3 machine.
- `-r` or `--report`: report previous experiment results (stored in the `results/` folder) without running the experiments.
