---
title: 'OrpailleCC: a Library for Data Stream Analysis on Embedded Systems'
tags:
  - C++
  - data stream
  - embedded systems
authors:
  - name: Martin Khannouz
    orcid: 0000-0003-2129-5517
    affiliation: 1
  - name: Bo Li
    orcid: 0000-0003-4207-5070 
    affiliation: 1
  - name: Tristan Glatard
    orcid: 0000-0003-2620-5883
    affiliation: 1
affiliations:
 - name: Department of Computer Science and Software Engineering, Concordia University, Montreal, Canada
   index: 1
date: 29 March 2019
bibliography: paper.bib
---

# Summary

The Internet of Things could benefit in several ways from mining data 
streams on connected objects rather than in the cloud. In particular, 
limiting network communication with cloud services would improve user 
privacy and reduce energy consumption in connected devices. Besides, 
applications could leverage the computing power of connected objects 
for improved scalability.

OrpailleCC provides a consistent collection of data stream algorithms 
developed to be deployed on embedded devices.  Its main objective is to
support research on data stream mining for connected objects,
by facilitating the comparison and benchmarking of algorithms in a 
consistent framework. It also enables programmers of embedded systems to use 
out-of-the-box algorithms with an efficient implementation.
To the best of our knowledge, existing libraries of stream mining
algorithms cannot be used on connected objects due to their resource consumption or
assumptions about the target system (e.g., existence of a `malloc` function).
Nevertheless, for more powerful devices such as desktop computers, Java
frameworks such as Massive Online Analysis [@moa] and WEKA [@weka] achieve
similar goals as OrpailleCC.

OrpailleCC targets the classes of problems discussed in [@kejariwal2015],
 in particular Sampling and 
Filtering. Sampling covers algorithms that 
build a representative sample of a
data stream. OrpailleCC implements the reservoir
sampling [@reservoir_sampling] and one variant, the chained reservoir
sampling [@chained_reservoir_sampling]. Filtering algorithms
remove the stream elements that do not belong to a specific set.
OrpailleCC implements the Bloom Filter [@bloom] and the Cuckoo
Filter [@cuckoo_filter], two well-tested algorithms that address this
problem.

In addition to Sampling and Filtering, OrpailleCC
provides algorithms for stream Classification and for stream Compression. The 
Micro-Cluster Nearest Neighbour algorithm [@mc-nn] is based on the 
k-nearest neighbor to classify a data stream while detecting concept 
drifts. The Lightweight Temporal Compression [@ltc] and a 
multi-dimensional variant [@ltcd] are two methods to compress data 
streams.

All implementations rely as little as possible on functions provided by the 
operating system, for instance `malloc`, since such functions are typically
not available on embedded systems. When algorithms cannot be
implemented without such functions, the library uses template parameters to 
request the required functions from the user.  All algorithms are 
developed for FreeRTOS [@freertos], a free real-time operating 
system used in embedded systems, but they should work on any 
micro-controller with a C++11 compiler. The C++11 programming language 
was chosen for its performance as well as its popularity in the 
field. All methods are tested and tests are run through Travis-CI.

In the future, we plan to extend the library with other reliable 
algorithms to widely cover as many common problems as possible. We also plan to 
use it as a basis to design new stream classification methods.
External contributions are, of course, most welcome.

# References
