#!/usr/bin/env python3

"""
Summarize the edges executed by an instrumented program.

Author: Adrian Herrera
"""


from argparse import ArgumentParser
from collections import defaultdict
import re
import sys

from tabulate import tabulate


REGEXES = {
    'function call': re.compile(r' function call '),
    'function return': re.compile(r' function return '),
    'conditional branch': re.compile(r' conditional branch '),
    'unconditional branch': re.compile(r' unconditional branch '),
    'switch': re.compile(r' switch '),
    'unknown edge': re.compile(r' unknown edge '),
}


def parse_args():
    """Parse command-line arguments."""
    parser = ArgumentParser(description='Summarize executed edges')
    parser.add_argument('log', nargs='+', help='Path to the edge log file(s)')

    return parser.parse_args()


def main():
    """The main function."""
    args = parse_args()

    # Collect results from logs
    results = defaultdict(lambda: defaultdict(int))

    for log_path in args.log:
        with open(log_path, 'r') as log:
            for line in log:
                for label, regex in REGEXES.items():
                    match = regex.search(line)
                    if match:
                        results[log_path][label] += 1
                        break

    # Tabulate results
    header = ('log', *REGEXES.keys())
    table = ((log, *[result[label] for label in REGEXES.keys()]) for
             log, result in results.items())
    print(tabulate(table, headers=header))


if __name__ == '__main__':
    main()
