"""Run weird machines to reporduce the results on the paper (section 5)"""

import argparse
import os
import subprocess
import time
import util
from typing import List, Dict

file_path = os.path.dirname(os.path.abspath(__file__))
exp_path = os.path.join(file_path, "../results")
MIN_DIV_ROUNDS = 1
MAX_DIV_ROUNDS = 50
MEASURE_ITER = 1000
MIN_EC_ACC = 0.95


def get_best_window(circuit: str, config: util.Config, fast: False) -> List[int]:
    """Get the best transient window size for a circuit"""
    circuit_path = os.path.join(file_path, "../build", circuit, circuit)
    best_size, EC_best_size = 0, 0
    best_acc, best_runtime = 0, 0

    # get the best window size without EC, skip if timeout for this circuit is 0
    if config.timeout != 0:
        for i in range(MIN_DIV_ROUNDS, MAX_DIV_ROUNDS + 1):
            util.rprint(
                f"Testing the best window size for {circuit}: {i}/{MAX_DIV_ROUNDS}"
            )
            for _ in range(5):
                acc, runtime = util.run_circuit(
                    circuit_path + f"-{i}.elf", 200 if fast else MEASURE_ITER, False, -1
                )
                if acc > best_acc or (acc == best_acc and runtime < best_runtime):
                    best_acc, best_runtime, best_size = acc, runtime, i

        print(
            (
                f"Best window size for {circuit}: {best_size} "
                + f"({best_acc:.2f}%, {best_runtime:.2f} us)"
            ).ljust(80)
        )

    # get the best window size with EC, skip if timeout is 0
    if config.EC_timeout == 0:
        return [best_size, 0]

    # test EC circuit with timeout that grows exponentially
    # this can tolerate circuits with large runtime
    timeout = 0.5
    for trial in range(10):
        best_acc, best_runtime = 0, 10**10
        for i in range(MIN_DIV_ROUNDS, MAX_DIV_ROUNDS + 1):
            filename = circuit_path + f"-{i}.elf"
            util.rprint(
                f"Testing best window for {circuit} (EC): "
                + f"{trial + 1} - {i}/{MAX_DIV_ROUNDS}"
            )

            # make sure the circuit is not extremely slow
            try:
                util.run_circuit(filename, config.test_iter // 10, True, timeout / 10)
            except subprocess.TimeoutExpired:
                continue

            # test the accuracy with full iterations
            try:
                acc, runtime = util.run_circuit(
                    filename, config.test_iter, True, timeout
                )
            except subprocess.TimeoutExpired:
                continue
            if acc > MIN_EC_ACC and runtime < best_runtime:
                best_acc, best_runtime, EC_best_size = acc, runtime, i

        if EC_best_size != 0:
            break
        timeout *= 2
        util.rprint(" " * 80)

    print(
        (
            f"Best window size for {circuit} (EC): {EC_best_size} "
            + f"({best_acc:.2f}%, {best_runtime:.2f} us)"
        ).ljust(80)
    )

    return [best_size, EC_best_size]


def measure_acc_runtime(circuit: str, config: util.Config, window_size: int, ec: bool):
    """Measure the accuracy and runtime of a circuit"""
    circuit_path = (
        os.path.join(file_path, "../build", circuit, circuit) + f"-{window_size}.elf"
    )
    result_path = os.path.join(
        exp_path, circuit + ("-EC" if ec else "") + f"-{window_size}.csv"
    )

    # create the result csv file if not exist
    if not os.path.exists(result_path):
        with open(result_path, "w") as ofile:
            ofile.write("Timestamp,Accuracy (%),Runtime (us)\n")

    # stop measuring when the timeout expires
    timeout = config.EC_timeout if ec else config.timeout
    end = (time.time() + timeout * 60) if timeout != -1 else None

    # run the circuit and append results to `result_path`
    trials = config.EC_measure_trials if ec else config.measure_trials
    for t in range(trials):
        util.rprint(
            f"Running {circuit}" + (" (EC)" if ec else "") + f": {t + 1}/{trials}"
        )
        try:
            left = None if end is None else (end - time.time())
            acc, runtime = util.run_circuit(circuit_path, MEASURE_ITER, ec, left)
            with open(result_path, "a") as ofile:
                ofile.write(f"{int(time.time())},{acc:.2f},{runtime:.2f}\n")
        except subprocess.TimeoutExpired:
            print(
                f"[!] Timeout expires while measuring {circuit}"
                + (" (EC)" if ec else "")
            )
            return

    print((f"[*] Done measuring {circuit}" + (" (EC)" if ec else "")).ljust(80))


def output_median(circuit: str, window_size: int, ec: bool) -> str:
    """Output the median accuracy and runtime of a circuit"""
    result_path = os.path.join(
        exp_path, circuit + ("-EC" if ec else "") + f"-{window_size}.csv"
    )
    if not os.path.exists(result_path):
        return ""

    acc, runtime = util.median_result(result_path, 2)
    output = circuit + (" (EC)" if ec else "") + ","
    output = output.ljust(20)
    output += f"{acc:.2f}%,".rjust(10)
    output += f"{runtime:.2f} us".rjust(15)
    return output + "\n"


def report(ckt_config: Dict[str, util.Config], best_window: Dict[str, List[int]]):
    """Output the median accuracy and runtime for each circuit"""
    output = "Circuit,".ljust(20) + "Accuracy,".rjust(10) + "Runtime".rjust(15) + "\n"
    for ckt in ckt_config:
        if ckt not in best_window:
            print(f"[!] Missing experiment results for {ckt}")
            continue

        output += output_median(ckt, best_window[ckt][0], False)
        output += output_median(ckt, best_window[ckt][1], True)

    print(output)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Run WM: measure the accuracy and runtime of weird machines."
    )
    parser.add_argument(
        "-f", "--fast", action="store_true", help="Run with fast timeouts"
    )
    parser.add_argument(
        "--simon25",
        action="store_true",
        help="Run Simon with 25 rounds instead of 32 rounds",
    )
    parser.add_argument(
        "-r",
        "--report",
        action="store_true",
        help="Report the existing results without running the circuits again",
    )
    args = parser.parse_args()

    # ensure config file exists
    config_file = os.path.join(
        file_path, "config-fast.csv" if args.fast else "config.csv"
    )
    if not os.path.exists(config_file):
        print(f"[!] Config file not found")
        exit(1)

    # parse config file
    raw_config = util.csv_to_dict(config_file)

    # use simon25 instead of simon32 is specified by the user
    if args.simon25 and "simon32" in raw_config:
        raw_config["simon25"] = raw_config.pop("simon32")

    # load config
    ckt_config = {}
    for ckt in raw_config:
        ckt_config[ckt] = util.Config(raw_config[ckt])

    # create the result directory if not exists
    if not os.path.exists(exp_path):
        os.mkdir(exp_path)

    # get or measure the best transient window size for each circuit
    window_file = os.path.join(exp_path, "best_window.csv")
    if os.path.exists(window_file):
        best_window = util.csv_to_dict(window_file)
    else:
        best_window = {}

    # report existing results without running experiments
    if args.report:
        report(ckt_config, best_window)
        exit()

    # find the best window for circuits and save the results to a csv file
    header = "circuit,best window size,best window size with error correction"
    for ckt, config in ckt_config.items():
        if ckt not in best_window:
            best_window[ckt] = get_best_window(ckt, config, args.fast)
            util.dict_to_csv(best_window, window_file, header)

    # measure the accuracy and runtime
    for ckt, config in ckt_config.items():
        # measure without error correction
        if config.timeout != 0:
            measure_acc_runtime(ckt, config, best_window[ckt][0], False)
        # measure with error correction
        if config.EC_timeout != 0:
            measure_acc_runtime(ckt, config, best_window[ckt][1], True)

    # report median accuracy and runtime
    report(ckt_config, best_window)
