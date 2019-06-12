[![Build Status](https://travis-ci.org/azazel7/OrpailleCC.svg?branch=master)](https://travis-ci.org/azazel7/OrpailleCC)

OrpailleCC is data stream library written in C++. It provides a consistent
collection of data stream algorithms for embedded devices. The goal of
OrpailleCC is to support research on data stream mining for connected objects,
by facilitating the comparison and benchmarking of algorithms in a consistent
framework. It also enables programmers of embedded systems to use
out-of-the-box algorithms with an efficient implementation.
Algorithms from OrpailleCC are based on C++ templates and does not use the STL library.

# Get started
## Hello World
Let us run a basic example with a reservoir sampling [4] of size 3.
Save the following code in *testy.cpp*.
```cpp
#include <iostream> //Included for cout
#include "reservoir_sampling.hpp"

double randy(void){ //We need this function to provide a random number generator to ReservoirSampling.
	return (double)rand() / (double)RAND_MAX; //On systems without rand, the programmer will have to define a pseudo-random function.
}

int main(){
	char hello[] = "Hello-world!"; //Create a stream
	ReservoirSampling<char, 3, randy> rs; //Instantiate a ReservoirSampling instance
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
## Install
### Requirement
As the collection is designed to run on embedded system without operating
systems, OrpailleCC has very little dependencies and requirement.

- Git : to download the repository.
- C++ compiler with C++11: to compile OrpailleCC files.
- googletest: to run unit tests.
- Linux Operating System: because all instructions are given for Linux systems. However, OrpailleCC should compile properly on a Windows system as long as a C++ compiler is available.

### Installation
To install OrpailleCC, first clone the repository. 
```bash
git clone https://github.com/big-data-lab-team/OrpailleCC.git
```
In this example, we assume that OrpailleCC is located in
`/usr/include/OrpailleCC`. Change it accordingly to your system.
```bash
ORPAILLECC_DIR=/usr/include/OrpailleCC
```

To use OrpailleCC in your project add `ORPAILLECC_DIR/src` in the include directories of the project.
Let's assume the project is the hello world example, located in *~/hello/hello.cpp*.

```cpp
#include <iostream> //Included for cout
#include <reservoir_sampling.hpp>

double randy(void){ //We need this function to provide a random number generator to ReservoirSampling.
	return (double)rand() / (double)RAND_MAX; //On systems without rand, the programmer will have to define a pseudo-random function.
}

int main(){
	char hello[] = "Hello-world!"; //Create a stream
	ReservoirSampling<char, 3, randy> rs; //Instantiate a ReservoirSampling instance
	//This instance works with char, contains a reservoir of size 3 and  use the randy function to generate random numbers.
	for(int j = 0; j < 12; ++j) //Feed the ReservoirSampling instance with every element of the stream (here letters of the string)
		rs.add(hello[j]);
	for(int j = 0; j < 3; ++j) //Print every element in the reservoir
		std::cout << rs[j];
	std::cout << std::endl;
	return 0;
}
```

To compile this code (that use the ReservoirSampling object), you need to run the following commands.

```bash
cd ~/hello
g++ -std=c++11 -I$ORPAILLECC_DIR hello.c
```

## Test
### Unit Test
The unit tests require the `googletest` library ([Here](https://github.com/google/googletest)).
To run the unit tests, run the command `make run_test`.

### Performance
To run a performance test on your device, compile the performance tests with
`make perf` then run `./main-perf`.

![Alt](/figures/performance.png "An example of the performance output")

### Coverage
To observe the coverage of test function, run the following commands:
```bash
make clean
make config=debug coverage
```
These commands will clean previous object files to rebuild them with the debug options, then run the test and gather the data for the coverage.
To visualize the test coverage, simply open *out/index.html* into your favorite browser.

# Examples
This section provides the list of all algorithms implemented in OrpailleCC with a brief example.
## Lightweight Temporal Compression (LTC)
LTC [0] is a compression algorithm that approximates a series of values with a linear
function. The epsilon parameter controls the amount of compression. If the
linear approximation isn't accurate enough, then a new point is
issued.

To use the LTC object, you need to include the header `ltc.hpp`.
```cpp
#include "ltc.hpp"

int main(){
	LTC<int, int, 3> comp; //Instantiate an LTC object that works with integers as element, and integers as timestamp.
	//The epsilon is set to 3. 
	for(int i = 0; i < 10000; ++i){ //10000 points are added sequentially.
		//The add function returns true, if the new point cannot be compressed with the previous ones and thus needs to be transmitted.
		bool need_transmission = comp.add(i, i%200);
		if(need_transmission){
			int timestamp, value;
			//the function get_value_to_transmit fetch the next data point to send and stores it into timestamp and value.
			comp.get_value_to_transmit(timestamp, value);
			// ... Execute the transmission or store the point.
		}
	}
}
```
## Micro-Cluster Nearest Neighbour (MC-NN)
MC-NN [3] is a classifier based on k-nearest neighbours. It aggregates the data
points into micro-clusters and make them evolve to catch concept drifts.

```cpp
#include <iostream> //Included for cout
#include "mc_nn.hpp"

#define MCNN_FEATURE_COUNT 2
int main(){
	int const dataset_size = 9;
	/*Instantiate a MCNN classifier that uses:
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
The next example is the one used as a hello world example. A Reservoir 
Sample [4] is a fixed-sized sample of the stream where all elements have 
equal probability to appear.
```cpp
#include <iostream> //Included for cout
#include "reservoir_sampling.hpp"

double randy(void){ //We need this function to provide a random number to ReservoirSampling.
	return (double)rand() / (double)RAND_MAX; //On systems without rand, the programmer will have to define a pseudo-random function.
}

int main(){
	char hello[] = "Hello-world!"; //Create a stream
	ReservoirSampling<char, 3, randy> rs; //Instantiate a ReservoirSampling instance
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
The chained reservoir sampling [1] is a variant of the reservoir sampling that allows discarding outdated data while maintaining the reservoir distribution.

```cpp
#include <iostream> //Included for cout
#include <cstdlib> //Included for malloc (note: that on other system, the malloc function may be redifined otherwise.)
#include "chained_reservoir.hpp"

//Define a structure that contains the malloc function
struct funct{
	static void* malloc(unsigned int const size){
		return std::malloc(size);
	}
};
//Declare the random function
double randy(void){
	return (double)rand() / (double)RAND_MAX;
}
#define RESERVOIR_SIZE 10
int main(){
	/*Create a ChainedReservoirSampling object:
		- That stores integer
		- Which has a reservoir size of 10 (RESERVOIR_SIZE)
		- Which uses randy as random function
		- Which uses the structure funct to call for other functions. (in that case, only memory allocation function)
	*/
	ChainedReservoirSampling<int, RESERVOIR_SIZE, randy, funct> rs;
	for(int i = 0; i < 100; ++i)
		rs.add(i, i+1); // Add new element to the reservoir (given a certain probability)
	std::cout << "Original reservoir: "; 
	for(int i = 0; i < RESERVOIR_SIZE; ++i)
		std::cout << rs[i] << " "; //Access the reservoir content
	std::cout << std::endl;
	rs.obsolete(20); //Declare all timestamp before 20 obsolete (20 is included)
	std::cout << "Obsolete reservoir: "; 
	for(int i = 0; i < RESERVOIR_SIZE; ++i)
		std::cout << rs[i] << " ";
	std::cout << std::endl;
}
```
## Bloom Filter
The Bloom filter [5] excludes elements from the stream when they don't belong to 
a pre-defined set.
```cpp
#include <iostream> //Included for cout
#include "bloom_filter.hpp"

#define BLOOM_FILTER_SIZE 50
unsigned int hash_int(int* element){
	return (*element)%BLOOM_FILTER_SIZE;
}
int main(){
	auto p = hash_int;
	/*Instantiate a BloomFilter object:
		- The element type taken as input (int)
		- The size (in bit) of the filter
		- The number of hash function to use
	With p containing the hash function.
	*/
	BloomFilter<int, BLOOM_FILTER_SIZE, 1> bf(&p);
	//Add 3 elements to the filter
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
The Cuckoo filter [2] is used when elements have to be removed from the pre-defined 
set of accepted elements.
```cpp
#include <iostream> //Included for cout
#include "cuckoo_filter.hpp"

#define CUCKOO_BUCKET_COUNT 32
#define CUCKOO_ENTRY_BY_BUCKET 6
#define CUCKOO_ENTRY_SIZE 3

//This structure contains functions for the cuckoo filter.
struct funct_cuckoo{
	//The fingerprint function should return value within the size of an entry size.
	static unsigned char fingerprint(int const* e){
		//mod 7 == value between 0 and 6, +1 == value between 1 and 7, so the empty value (0x0) is avoided
		return ((*e)%7)+1; 
	}
	//The hash function that takes an element as input and return an index.
	static unsigned int hash(int const* e){
		return (*e)%CUCKOO_BUCKET_COUNT;		
	}
	//This hash function take a fingerprint as input and return an index.
	static unsigned int hash(unsigned char fingerprint){
		//Here, we make sure that the two hashfunctions does not return the same value
		return (fingerprint * 7)%CUCKOO_BUCKET_COUNT;
	}
};
double randy(void){ //We need this function to provide a random number to CuckooFilter.
	return (double)rand() / (double)RAND_MAX; //On systems without rand, the programmer will have to define his own function.
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

# How can I help?
- Report issues and seek support in the Issues tab.
- Write new examples or improve existing examples and share them with a pull request. 
- Submit ideas for future algorithms to integrate.
- Submit pull requests with algorithm implementation.
- Submit pull requests with additional test cases.

# References
- [0] Schoellhammer, Tom and Greenstein, Ben and Osterweil, Eric and Wimbrow, Michael and Estrin, Deborah (2004), "Lightweight temporal compression of microclimate datasets"
- [1] Babcock, Brian and Datar, Mayur and Motwani, Rajeev (2002), "Sampling from a moving window over streaming data", Proceedings of the thirteenth annual Association for Computing Machinery-SIAM symposium on Discrete algorithms, pages 633--634
- [2] Fan, Bin and Andersen, Dave G and Kaminsky, Michael and Mitzenmacher, Michael (2014), "Cuckoo filter: Practically better than bloom", Proceedings of the 10th Association for Computing Machinery International on Conference on emerging Networking Experiments and Technologies, pages 75--88
- [3] Tennant, Mark and Stahl, Frederic and Rana, Omer and Gomes, Joao Bartolo (2017), "Scalable real-time classification of data streams with concept drift", Future Generation Computer Systems, pages 187--199
- [4] Vitter, Jeffrey S (1985), "Random sampling with a reservoir", Association for Computing Machinery Transactions on Mathematical Software (TOMS), pages 37--57
- [5] Burton H. Bloom (1970), "Space/Time Trade-offs in Hash Coding with Allowable Errors", Communications of the Association for Computing Machinery
