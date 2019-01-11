[![Build Status](https://travis-ci.org/azazel7/OrpailleCC.svg?branch=master)](https://travis-ci.org/azazel7/OrpailleCC)

OrpailleCC is data stream library writen in C++. It provide a collection of algorithm for embeded device such as sensors.

The library is based on template and does not use the STL library.
To start using a feature, just include the header files in your project and compile your project.

* Get started

** Use algorithms
Simply pick the code you need and add to your project.  You also need to add
the C++11 (`-std=c++11`) flag to your compiling toolchain.

** Test
*** Unit Test
To run the unit tests, run the command `make run_test`.

*** Performance
To run a performance test on your laptop, compile the performance tests with
`make perf` then run `./main-perf`.

![Alt](/figures/performance.png "An example of the performance output")

* List of Algorithms

- Lightweight Temporal Compression
- Micro-Cluster Nearest Neighbour
- Reservoir Sampling
- Chained Reservoir Sampling
- Bloom Filter
- Cuckoo Filter

* Example
** Lightweight Temporal Compression
Add example
** Micro-Cluster Nearest Neighbour
Add example
** Reservoir Sampling
Add example
** Chained Reservoir Sampling
Add example
** Bloom Filter
Add example
** Cuckoo Filter
Add example
