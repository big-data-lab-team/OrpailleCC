#include <stdio.h>
#include <stdlib.h>
#include <typeinfo>
#include "bloom_filter.hpp"
#include "cuckoo_filter.hpp"
#include "reservoir_sampling.hpp"

int main(){
	ReservoirSampling<int, 10> rs;
	rs.add(3);
	printf("%i\n", rs[0]);
}
