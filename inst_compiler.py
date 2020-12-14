#!/usr/bin/env python3

"""
Insert LLVM instrumentation.

Author: Adrian Herrera
"""


import os
from pathlib import Path
from shutil import which
from subprocess import run
import sys


DIR = Path(__file__).parent
LIB_DIR = DIR.parent  / 'lib'


def main():
    """The main function."""
    args = sys.argv
    env = os.environ.copy()

    # Determine compiler
    if sys.argv[0].endswith('++'):
        cc = env.get('LLVM_CXX')
        if not cc:
            cc = which('clang++')
    else:
        cc = env.get('LLVM_CC')
        if not cc:
            cc = which('clang')
    if not cc:
        raise Exception('Unable to find clang (set LLVM_CC env variable)')

    if env.get('LLVM_SPLIT_COMPARES'):
        plugins = (LIB_DIR / 'split-compares.so',
                   LIB_DIR / 'split-switches.so',
                   LIB_DIR / 'edge-log.so')
    else:
        plugins = (LIB_DIR / 'edge-log.so',)

    plugin_opts = ['-fplugin=%s' % plug.resolve() for plug in plugins]

    # Determine build flags
    bit_mode = 32 if '-m32' in args else 64
    maybe_linking = True
    if any(arg in args for arg in ('-v', '-c', '-S', '-E')):
        maybe_linking = False

    # Run the build
    run_args = [cc, *plugin_opts, '-Qunused-arguments']
    if len(args) > 1:
        run_args.extend([*args[1:]])
    if maybe_linking:
        run_args.extend(['-lstdc++', '-ldl', '-lz',
                         '-L%s' % LIB_DIR, '-ledge-log-rt-%d' % bit_mode])
    proc = run(run_args, env=env, check=False)

    return proc.returncode


if __name__ == '__main__':
    sys.exit(main())
