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
```cpp
#include <iostream> //Included for cout
#include "mc_nn.hpp"

#define MCNN_FEATURE_COUNT 2
int main(){
	int const dataset_size = 9;
	/*Instanciate a MCNN classifier that uses:
		- double as features
		- which consider two features (MCNN_FEATURE_COUNT)
		- That uses 10 clusters at most
	*/
	MCNN<double, MCNN_FEATURE_COUNT, 10> classifier;
	//Initialize a fake dataset
	double dataset[][MCNN_FEATURE_COUNT] = 
						   {{ 0,  0},
							{ 1,  1},
							{ 8,  1},
							{ 1,  8},
							{ 7,  2},
							{ 2,  8},
							{-1,  0},
							{ 1,  0},
							{ 7, -1}};
	int labels[] = {1, 1, 2, 2,2,2,1,1,2};

	//Train the classifier
	for(int i = 0; i < dataset_size; ++i){
		classifier.train(dataset[i], labels[i]);
	}
	double test1[MCNN_FEATURE_COUNT] = {8, 0};
	double test2[MCNN_FEATURE_COUNT] = {-1, -1};
	int prediction1 = classifier.predict(test1);
	int prediction2 = classifier.predict(test2);
	std::cout << "Prediction { 8,  0} :" << prediction1 << std::endl;
	std::cout << "Prediction {-1, -1} :" << prediction2 << std::endl;
	return 0;
}
```
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
```cpp
#include <iostream> //Included for cout
#include "bloom_filter.hpp"

#define BLOOM_FILTER_SIZE 50
unsigned int hash_int(int* element){
	return (*element)%BLOOM_FILTER_SIZE;
}
int main(){
	auto p = hash_int;
	/*Instanciate a BloomFilter object:
		- The element type taken as input (int)
		- The size (in bit) of the filter
		- The number of hash function to use
	With p containing the hash function.
	*/
	BloomFilter<int, BLOOM_FILTER_SIZE, 1> bf(&p);
	//Add 3 element to the filter
	bf.add(10);
	bf.add(7);
	bf.add(73);
	std::cout << "Element accepted by the bloom filter: ";
	for(int i = 0; i < 100; ++i){
		if(bf.lookup(i)){
			std::cout << i << " ";
		}
	}
	std::cout << std::endl;
}
```
Note that, due to the Bloom Filter size, more than three elements will be recognized by the filter.

## Cuckoo Filter
```cpp
#include <iostream> //Included for cout
#include "cuckoo_filter.hpp"

#define CUCKOO_BUCKET_COUNT 32
#define CUCKOO_ENTRY_BY_BUCKET 6
#define CUCKOO_ENTRY_SIZE 3

//This structure contains functions for the cuckoo filter.
struct funct_cuckoo{
	//The finger print function should return 
	static unsigned char fingerprint(int const* e){
		//mod 7 == value between 0 and 6, +1 == value between 1 and 7, so the empty value (0x0) is avoided
		return ((*e)%7)+1; 
	}
	static unsigned int hash(int const* e){
		return (*e)%CUCKOO_BUCKET_COUNT;		
	}
	/*
	 * Make the combination of hash element and hash fingerprint does not lead to the same value
	 * Hash of fingerprint should probably not return a 0 or a 2^CUCKOO_ENTRY_SIZE-1 because it create same h1 and h2
	 */
	static unsigned int hash(unsigned char fingerprint){
		return (fingerprint * 7)%CUCKOO_BUCKET_COUNT;
	}
};
double randy(void){ //We need this function to provide a random number to CuckooFilter.
	return (double)rand() / (double)RAND_MAX; //On systems without rand, the programmer will have to define a pseudo-random function.
}
int main() { 
	CuckooFilter<int, CUCKOO_BUCKET_COUNT, CUCKOO_ENTRY_BY_BUCKET, CUCKOO_ENTRY_SIZE, funct_cuckoo, randy> cf;
	cf.add(10);
	cf.add(7);
	cf.add(73);
	std::cout << "Element accepted by the cuckoo filter: ";
	for(int i = 0; i < 100; ++i){
		if(cf.lookup(i)){
			std::cout << i << " ";
		}
	}
	std::cout << std::endl;
}
```
