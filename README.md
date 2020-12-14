# LLVM Edge Log

Instruments a program such that edges can be **precisely** logged at runtime (as
opposed to AFL, which uses a lossy hash function).

## Requirements

* Modern-ish LLVM (>= 7.0)
* libz development files

## Building

```console
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install
make -j
make install
```

## Instrumenting

The `inst_compiler` wrapper can be used as a drop-in replacement for clang
(similarly, `inst_compiler++` can be used as a drop-in replacement for clang++).

E.g.,:

```console
/path/to/install/inst_compiler test.c
```

## Running

The following runtime options are available, specified via environment
variables:

* `EDGE_LOG_PATH`: Path to the output CSV file where executed edges will be
  written to.
* `EDGE_LOG_GZIP`: Set to compress output log using gzip (takes longer, but
  produces a smaller log file).
