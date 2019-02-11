[![Build Status](https://travis-ci.org/azazel7/OrpailleCC.svg?branch=master)](https://travis-ci.org/azazel7/OrpailleCC)

OrpailleCC is data stream library writen in C++. It provide a consistent
collection of datastream algorithm for embeded device such as sensors.

The library is based on template and does not use the STL library.  To start
using a feature, just include the header files in your project and compile your
project.

# Get started
## Hello World
Let run a basic example with a Reservoir sampling.
Save the following code in *testy.cpp*.
```cpp
#include <iostream> //Included for cout
#include "reservoir_sampling.hpp"

double randy(void){ //We need this function to provide a random number to ReservoirSampling.
	return (double)rand() / (double)RAND_MAX; //On systems without rand, the programmer will have to define a pseudo-random function.
}

int main(){
	char hello[] = "Hello-world!"; //Create a stream
	ReservoirSampling<char, 3, randy> rs; //Instanciate a ReservoirSampling instance
	//This instance works with char, contains a reservoir of size 3 and  use the randy function to generate random numbers.
	for(int j = 0; j < 12; ++j) //Feed the ReservoirSampling instance with every element of the stream (here letters of the string)
		rs.add(hello[j]);
	for(int j = 0; j < 3; ++j) //Print every element in the reservoir
		std::cout << rs[j];
	std::cout << std::endl;
	return 0;
}
```

Then compile it with your favorite C++ compiler and run it.
```bash
$ g++ -I./src -std=c++11 testy.cpp -o testy
$ ./testy
Hll
```
## Use the library in your project
Simply pick the code you need and add to your project.  You also need to add
the C++11 (`-std=c++11`) flag to your compiling toolchain.

An alternative is to add `<OrpailleCC dir>/src` to the include paths of your compiler.

## Test
### Unit Test
The unit tests require the `googletest` library.
To run the unit tests, run the command `make run_test`.

### Performance
To run a performance test on your laptop, compile the performance tests with
`make perf` then run `./main-perf`.

![Alt](/figures/performance.png "An example of the performance output")

# List of Algorithms

- Lightweight Temporal Compression
- Micro-Cluster Nearest Neighbour
- Reservoir Sampling
- Chained Reservoir Sampling
- Bloom Filter
- Cuckoo Filter

# Example
## Lightweight Temporal Compression (LTC)
To use the LTC object, you need to include the header `ltc.hpp`.
```cpp
#include "ltc.hpp"

int main(){
	LTC<int, int, 3> comp; //Instanciate an LTC object that works with integer as element, and integer as timestamp.
	//The epsilon is set to 3. 
	for(int i = 0; i < 10000; ++i){ //10000 points are added sequentially.
		//The add function return true, if the new point cannot be compressed with the previous ones and thus needs to be transmitted.
		bool need_transmission = comp.add(i, i%200);
		if(need_transmission){
			int timestamp, value;
			//the function get_value_to_transmit fetch the next datapoint to send and stores it into timestamp and value.
			comp.get_value_to_transmit(timestamp, value);
			// ... Execute the transmission or store the point.
		}
	}
}
```
## Micro-Cluster Nearest Neighbour
Add example
## Reservoir Sampling
The next example is the one used as a hello world example.
```cpp
#include <iostream> //Included for cout
#include "reservoir_sampling.hpp"

double randy(void){ //We need this function to provide a random number to ReservoirSampling.
	return (double)rand() / (double)RAND_MAX; //On systems without rand, the programmer will have to define a pseudo-random function.
}

int main(){
	char hello[] = "Hello-world!"; //Create a stream
	ReservoirSampling<char, 3, randy> rs; //Instanciate a ReservoirSampling instance
    //This instance works with char, contains a reservoir of size 3 and  use the randy function to generate random numbers.
	for(int j = 0; j < 12; ++j) //Feed the ReservoirSampling instance with every element of the stream (here letters of the string)
		rs.add(hello[j]);
	for(int j = 0; j < 3; ++j) //Print every element in the reservoir
		std::cout << rs[j];
	std::cout << std::endl;
	return 0;
}
```
## Chained Reservoir Sampling
Add example
## Bloom Filter
Add example
## Cuckoo Filter
Add example
