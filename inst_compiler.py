#!/usr/bin/env python3

"""
Insert LLVM instrumentation.

Author: Adrian Herrera
"""


import os
from shutil import which
from subprocess import run
import sys


DIR = os.path.dirname(__file__)
LIB_DIR = os.path.join(DIR, '..', 'lib')


def main():
    """The main function."""
    args = sys.argv

    # Determine compiler
    if sys.argv[0].endswith('++'):
        cc = os.environ.get('LLVM_CXX')
        if not cc:
            cc = which('clang++')
    else:
        cc = os.environ.get('LLVM_CC')
        if not cc:
            cc = which('clang')

    if os.environ.get('LLVM_SPLIT_COMPARES'):
        plugins = (os.path.join(LIB_DIR, 'split-compares.so'),
                   os.path.join(LIB_DIR, 'split-switches.so'),
                   os.path.join(LIB_DIR, 'edge-log.so'))
    else:
        plugins = (os.path.join(LIB_DIR, 'edge-log.so'))

    plugin_opts = ['-fplugin=%s' % os.path.realpath(plug) for plug in plugins]

    # Run the build
    run_args = [cc, *plugin_opts, '-Qunused-arguments', os.path.join(LIB_DIR, 'objects', 'edge-log-rt', 'edge-log-rt.c.o')]
    if len(args) > 1:
        run_args.extend([*args[1:]])
    print(' '.join(run_args))
    proc = run(run_args, check=False)

    return proc.returncode


if __name__ == '__main__':
    sys.exit(main())
