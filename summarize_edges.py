#!/usr/bin/env python3

"""
Summarize the edges executed by an instrumented program.

Author: Adrian Herrera
"""


from argparse import ArgumentParser, Namespace
from collections import defaultdict
from csv import DictReader, DictWriter
from pathlib import Path

from tabulate import tabulate


def parse_args() -> Namespace:
    """Parse command-line arguments."""
    parser = ArgumentParser(description='Summarize executed edges')
    parser.add_argument('-c', '--csv', required=False, type=Path,
                        help='Path to output CSV')
    parser.add_argument('log', nargs='+', type=Path,
                        help='Path to the edge log file(s)')
    return parser.parse_args()


def main():
    """The main function."""
    args = parse_args()

    # Collect results from logs
    results = defaultdict(lambda: defaultdict(int))

    for log_path in args.log:
        with open(log_path, 'r') as log:
            reader = DictReader(log)
            for row in reader:
                results[log_path][(row['prev_addr'], row['cur_addr'])] += 1

    # Print results
    header = ('log', 'prev addr', 'cur addr', 'count')
    csv_path = args.csv
    if csv_path:
        with open(csv_path, 'w') as csvfile:
            writer = DictWriter(csvfile, fieldnames=header)
            writer.writeheader()
            writer.writerows(({'log': str(log),
                               **result,
                               'total': sum(result.values())} for log, result in
                              results.items()))
    else:
        table = ((log, *[result[label] for label in REGEXES],
                  sum(result.values())) for log, result in results.items())
        print(tabulate(table, headers=header))


if __name__ == '__main__':
    main()
