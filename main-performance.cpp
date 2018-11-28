#include <unistd.h>
#include <iostream>
#include "bloom_filter.hpp"
#include "cuckoo_filter.hpp"
#include <sys/time.h>
using namespace std;

#define BLOOM_FILTER_SIZE 600
unsigned int hash_int(int* element){
	return (*element)%BLOOM_FILTER_SIZE;
}
double When() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return ((double) tp.tv_sec + (double) tp.tv_usec * 1e-6);
}

#define CUCKOO_BUCKET_COUNT 32
#define CUCKOO_ENTRY_BY_BUCKET 6
#define CUCKOO_ENTRY_SIZE 3
struct funct_cuckoo{
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
double randy(void){
	return (double)rand() / (double)RAND_MAX;
}

void test_cuckoo(void) { 
	CuckooFilter<int, CUCKOO_BUCKET_COUNT, CUCKOO_ENTRY_BY_BUCKET, CUCKOO_ENTRY_SIZE, funct_cuckoo, randy> cf;
	unsigned int const count_add = 100000000;
	unsigned int const count_lookup = 100000000;
	double start = When();
	for(int i = 0; i < count_add; ++i){
		cf.add(i);
		if(i%100 == 0)
			cf.clear();
	}
	double stop = When();
	cout << "Time (Insert): " << (stop - start) << " (" << (((stop - start) / count_add) * 1e9) << " ns/item)" << endl;

	cf.clear();
	start = When();
	for(int i = 0; i < count_lookup; ++i){
		cf.lookup(i);
	}
	stop = When();
	cout << "Time (Lookup): " << (stop - start) << " (" << (((stop - start) / count_lookup) * 1e9) << " ns/item)" << endl;
}
void test_bloom(void){
	auto p = hash_int;
	unsigned int const count_add = 1000000000;
	unsigned int const count_lookup = 1000000000;
	BloomFilter<int, BLOOM_FILTER_SIZE, 1> bf(&p);
	double start = When();
	for(int i = 0; i < count_add; ++i){
		bf.add(i);
	}
	double stop = When();
	cout << "Time (Insert): " << (stop - start) << " (" << (((stop - start) / count_add) * 1e9) << " ns/item)" << endl;

	bf.clear();
	start = When();
	for(int i = 0; i < count_lookup; ++i){
		bf.lookup(i);
	}
	stop = When();
	cout << "Time (Lookup): " << (stop - start) << " (" << (((stop - start) / count_lookup) * 1e9) << " ns/item)" << endl;
}
int main(int argc, char** argv){
	test_bloom();
	test_cuckoo();
	return 0;
}
