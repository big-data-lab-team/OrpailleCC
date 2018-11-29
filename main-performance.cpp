#include <unistd.h>
#include <iostream>
#include "bloom_filter.hpp"
#include "cuckoo_filter.hpp"
#include "ltc.hpp"
#include "reservoir_sampling.hpp"
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
	cout << "\t=== Cuckoo ===" << endl;
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
	cout << "\t=== Bloom ===" << endl;
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
#define LTC_SIZE 1000000
void test_ltc(void){
	cout << "\t=== LTC ===" << endl;
	double start, stop;
	int count_smooth = 0, count_rough = 0, count_linear = 0;
	int vals_smooth[LTC_SIZE];
	int vals_rough[LTC_SIZE];
	for(int i = 0; i < LTC_SIZE; ++i){
		vals_smooth[i] = round(cos(i*0.1) * 20);
		vals_rough[i] = round(cos(i) * 20);
	}

	start = When();
	for(int j = 0; j < 1000; ++j)
	{
		LTC<3> comp;
		for(int i = 0; i < LTC_SIZE; ++i){
			bool a = comp.add(i, vals_rough[i]);
			if(a)
				count_rough += 1;
		}
	}
	stop = When();
	cout << "Time (rough): " << (stop - start) << " (" << (((stop - start) / (LTC_SIZE * 1000)) * 1e9) << " ns/item)" << endl;
	start = When();
	for(int j = 0; j < 1000; ++j)
	{
		LTC<3> comp;
		for(int i = 0; i < LTC_SIZE; ++i){
			bool a = comp.add(i, vals_smooth[i]);
			if(a)
				count_smooth += 1;
		}
	}
	stop = When();
	cout << "Time (smooth): " << (stop - start) << " (" << (((stop - start) / (LTC_SIZE * 1000)) * 1e9) << " ns/item)" << endl;
	start = When();
	for(int j = 0; j < 1000; ++j)
	{
		LTC<3> comp;
		for(int i = 0; i < LTC_SIZE; ++i){
			bool a = comp.add(i, i%200);
			if(a)
				count_linear += 1;
		}
	}
	stop = When();
	cout << "Time (linear): " << (stop - start) << " (" << (((stop - start) / (LTC_SIZE * 1000)) * 1e9) << " ns/item)" << endl;
	cout << "Rough: " << ((count_rough)/1e6) << "M" << endl;
	cout << "Smooth: " << ((count_smooth)/1e6) << "M" << endl;
	cout << "Linear: " << ((count_linear)/1e6) << "M" << endl;
}
void test_reservoir_sampling(void){
	cout << "\t=== Reservoir Sampling ===" << endl;
	ReservoirSampling<int, 100, randy> rs;
	unsigned int sum_pre_count = 0;
	int pre_count[100] = {0}, count_loop = 1000000;
	pre_count[20] = 1000;
	pre_count[25] = 500;
	pre_count[50] = 400;
	pre_count[75] = 250;
	for(int i = 0; i < 100; ++i)
		sum_pre_count += pre_count[i];

	int count[100] = {0};
	double start, stop;
	start = When();
	for(int k = 0; k < count_loop; ++k)
		for(int i = 0; i < 100; ++i)
			for(int j = 0; j < pre_count[i]; ++j)
				rs.add(i);
	stop = When();
	cout << "Time: " << (stop - start) << " (" << (((stop - start) / (count_loop * sum_pre_count)) * 1e9 ) << " ns/item)" << endl;
	for(int i = 0; i < 100; ++i){
		int idx = rs[i];
		if(idx >= 0 && idx < 100)
			count[idx] += 1;
	}
	for(int i = 0; i < 100; ++i)
		if(count[i] < 0)
			cout << i << ": " << count[i] << endl;
}
int main(int argc, char** argv){
	test_reservoir_sampling();
	test_ltc();
	test_bloom();
	test_cuckoo();
	return 0;
}
