"""Run packed programs to reporduce the results on the paper (section 6)"""

import argparse
import os
import subprocess
import time
import util
from typing import List, Dict, Tuple

file_path = os.path.dirname(os.path.abspath(__file__))
exp_path = os.path.join(file_path, "../results")
MEASURE_ITER = 100
MAX_TIMEOUT = 3600


def measure_runtime(circuit: str, window_size: int, timeout: int):
    """Measure the runtime of a packed program"""
    circuit_path = (
        os.path.join(file_path, "../build", circuit, circuit) + f"-{window_size}.elf"
    )
    result_path = os.path.join(exp_path, circuit + f"-{window_size}.csv")

    # create the result csv file if not exist
    if not os.path.exists(result_path):
        with open(result_path, "w") as ofile:
            ofile.write("Timestamp,Runtime (s)\n")

    # stop measuring when the timeout expires
    end = (time.time() + timeout * 60) if timeout != -1 else None

    # run the circuit and append results to `result_path`
    for t in range(MEASURE_ITER):
        util.rprint(f"Running {circuit}: {t + 1}/{MEASURE_ITER}")
        try:
            left = None if end is None else (end - time.time())
            single_timeout = left if left is None or left < MAX_TIMEOUT else MAX_TIMEOUT
            begin = time.time()
            subprocess.run(
                circuit_path,
                stdout=subprocess.DEVNULL,
                check=True,
                timeout=single_timeout,
            )
            with open(result_path, "a") as ofile:
                ofile.write(f"{int(time.time())},{time.time() - begin:.2f}\n")
        except subprocess.TimeoutExpired:
            print(f"[!] Timeout expires while measuring {circuit}")
            if left is not None and left < MAX_TIMEOUT:
                return
        except subprocess.CalledProcessError:
            print(f"[!] {circuit} unpacking failed".ljust(80))

    print(f"[*] Done measuring {circuit}".ljust(80))


def output_median(circuit: str, window_size: int) -> str:
    """Output the median runtime of a packed program"""
    result_path = os.path.join(exp_path, circuit + f"-{window_size}.csv")
    if not os.path.exists(result_path):
        return ""

    runtime = util.median_result(result_path, 1)[0]
    output = f"{circuit},".ljust(15)
    output += f"{runtime:.2f} s".rjust(15)
    return output + "\n"


def report(circuits: List[Tuple[str, int, str]], best_window: Dict[str, List[int]]):
    output = "Circuit,".ljust(15) + "Runtime".rjust(15) + "\n"
    for packer, _, ckt in circuits:
        if ckt not in best_window:
            print(f"[!] Missing experiment results for {ckt}")
            continue

        output += output_median(binary + "-" + packer, best_window[ckt][1])

    print(output)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Run packed: measure the runtime of a program packed by UPFlexo."
    )
    parser.add_argument(
        "--aes-timeout",
        action="store",
        help="Set the timeout of AES in minutes (default: 120)",
        default=120,
        type=int,
    )
    parser.add_argument(
        "--simon-timeout",
        action="store",
        help="Set the timeout of Simon in minutes (default: 120)",
        default=120,
        type=int,
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

    # create the result directory if not exists
    if not os.path.exists(exp_path):
        os.mkdir(exp_path)

    fast = False
    binary = "ls"
    circuits = [
        (
            "AES_CTR",
            args.aes_timeout,
            "aes_block",
        ),
        (
            "simon25_CTR" if args.simon25 else "simon32_CTR",
            args.simon_timeout,
            "simon25" if args.simon25 else "simon32",
        ),
    ]

    # get or measure the best transient window size for each circuit
    window_file = os.path.join(exp_path, "best_window.csv")
    if os.path.exists(window_file):
        best_window = util.csv_to_dict(window_file)
    else:
        print("[!] best_window.csv not found in results folder.")
        print("[!] Please run `run_WM.py` before running this script.")
        exit(1)

    # report existing results without running experiments
    if args.report:
        report(circuits, best_window)
        exit()

    # measure runtime
    for packer, timeout, ckt in circuits:
        if ckt not in best_window:
            print(f"[!] The best window size of circuit {ckt} is not recorded")
            print("[!] Please run `run_WM.py` to determine the best window size")
            continue
        measure_runtime(binary + "-" + packer, best_window[ckt][1], timeout)

    # report median accuracy and runtime
    report(circuits, best_window)
