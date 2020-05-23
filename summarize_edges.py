#!/usr/bin/env python3

"""
Summarize the edges executed by an instrumented program.

Author: Adrian Herrera
"""


from argparse import ArgumentParser
from collections import defaultdict
from csv import DictWriter
import re
import sys

from tabulate import tabulate


REGEXES = {
    'direct call': re.compile(r' direct call '),
    'indirect call': re.compile(r' indirect call '),
    'return': re.compile(r' return '),
    'conditional branch': re.compile(r' conditional branch '),
    'unconditional branch': re.compile(r' unconditional branch '),
    'switch': re.compile(r' switch '),
    'unknown edge': re.compile(r' unknown edge '),
}


def parse_args():
    """Parse command-line arguments."""
    parser = ArgumentParser(description='Summarize executed edges')
    parser.add_argument('-c', '--csv', required=False,
                        help='Path to output CSV')
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

    # Print results
    header = ('log', *REGEXES.keys())
    csv_path = args.csv
    if csv_path:
        with open(csv_path, 'w') as csvfile:
            writer = DictWriter(csvfile, fieldnames=header)
            writer.writeheader()
            writer.writerows(({'log': log, **result} for log, result in
                              results.items()))
    else:
        table = ((log, *[result[label] for label in REGEXES.keys()]) for
                 log, result in results.items())
        print(tabulate(table, headers=header))


if __name__ == '__main__':
    main()
