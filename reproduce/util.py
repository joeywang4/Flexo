import statistics
import subprocess
import sys
from typing import Dict, List, Tuple


class Config:
    """Circuit config data"""

    def __init__(self, raw_config: List[int]) -> None:
        assert len(raw_config) >= 4, "Invalid config data"
        self.test_iter = raw_config[0]
        self.measure_trials = raw_config[1]
        self.timeout = raw_config[2]
        self.EC_timeout = raw_config[3]


def csv_to_dict(filename: str) -> Dict[str, List[int | float]]:
    """
    Load a csv file and convert it to a dictionary.
    The first column is treated as the dict key, and the other
    columns are stored as list[int | float] in the dict
    """
    with open(filename, "r") as infile:
        lines = infile.readlines()[1:]

    output = {}
    for line in lines:
        data = line.split(",")
        if len(data) <= 1:
            continue

        nums = [(float(n) if "." in n else int(n)) for n in data[1:]]
        output[data[0]] = nums

    return output


def dict_to_csv(data: Dict[str, List[int | float]], filename: str, header: str = ""):
    """
    Save a dictionary to a csv file.
    The key of the dictionary is written to the first column, and the value,
    a list of int or float, is stored in the other columns.
    """
    output = header + "\n"

    for k, v in data.items():
        output += k + ","
        output += ",".join([(str(n) if isinstance(n, int) else f"{n:.2f}") for n in v])
        output += "\n"

    with open(filename, "w") as ofile:
        ofile.write(output)


def parse_output(output: str) -> Tuple[float, float]:
    """Parse execution result and return accuracy and time usage"""
    lines = output.splitlines()

    i = 0
    while i < len(lines) - 2:
        if lines[i].find("===") == -1:
            i += 1
            continue

        # parse the output of a gate, such as:
        # === AND gate ===
        # Accuracy: 95.11000%, Error detected: 4.81000%, Undetected error: 0.08000%
        # Time usage: 0.896 (us)
        # over 10000 iterations.
        beg = lines[i + 1].find("= (")
        beg = beg + 3 if beg != -1 else lines[i + 1].find(": ") + 2
        acc = float(lines[i + 1][beg : lines[i + 1].find("%")])
        sec = float(
            lines[i + 2][lines[i + 2].find(": ") + 2 : lines[i + 2].find("(us)")]
        )

        return (acc, sec)

    assert False, "Invalid output from a WM:\n" + output


def run_circuit(
    circuit: str, iters: int, ec: bool, timeout: int | float = -1
) -> Tuple[float, float]:
    """
    Measure the accuracy and runtime of a circuit.
    This function executes a circuit for `iters` times to measure the
    accuracy and runtime.
    When timeout is -1, there is no timeout when running the circuit.
    The return value is a tuple of two floats, (accuracy (%), runtime (us)).
    """
    if timeout == 0:
        return [0.0, 0.0]

    cmd = [circuit, "-t", str(iters)]
    if ec:
        cmd.append("-r")

    proc = subprocess.run(
        cmd,
        stdout=subprocess.PIPE,
        check=False,
        timeout=(timeout if timeout != -1 else None),
    )
    return parse_output(proc.stdout.decode())


def median_result(filename: str) -> Tuple[float, float]:
    """Read a experiment result file and output the median accuracy and runtime"""
    with open(filename, "r") as infile:
        lines = infile.readlines()[1:]
    
    accs, runtimes = [], []
    for line in lines:
        tokens = line.split(",")
        if len(tokens) < 3: continue
        accs.append(float(tokens[1]))
        runtimes.append(float(tokens[2]))
    print(filename)
    return (statistics.median(accs), statistics.median(runtimes))


def rprint(text: str):
    """Print text and then print `\r`"""
    print(text, end="\r", file=sys.stdout, flush=True)
