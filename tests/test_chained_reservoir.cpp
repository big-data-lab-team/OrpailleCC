#include <cstdlib>
#include "gtest/gtest.h"
#include "chained_reservoir.hpp"
using namespace std;
struct funct{
	static void* malloc(unsigned int const size){
		return std::malloc(size);
	}
	static double random(void){
		return static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
	}
};
TEST(ChainedReservoirSampling, Add) { 
	ChainedReservoirSampling<int, 100, funct> rs;
	int count[100] = {0};
	for(int i = 0; i < 100; ++i)
		rs.add(i, i+1);
	for(int i = 0; i < 100; ++i){
		int idx = rs[i];
		if(idx >= 0 && idx < 100)
			count[idx] += 1;
	}
	for(int i = 0; i < 100; ++i)
		EXPECT_EQ (1 , count[i]);
}
TEST(ChainedReservoirSampling, Add2) { 
	ChainedReservoirSampling<int, 100, funct> rs;
	int count[101] = {0};
	for(int i = 0; i < 101; ++i)
		rs.add(i, i+1);
	for(int i = 0; i < 100; ++i){ //Loop over the size of the sample, which is 100
		int idx = rs[i];
		if(idx >= 0 && idx < 101)
			count[idx] += 1;
	}
	int count_false = 0, count_true = 0, count_other = 0;
	for(int i = 0; i < 101; ++i){
		if(count[i] == 0)
			count_false += 1;
		else if(count[i] == 1)
			count_true += 1;
		else
			count_other += 1;
	}
	EXPECT_EQ (100, count_true); //The size of the sample is 100, so we should have 100 elements in the sample
	EXPECT_EQ (1, count_false); //We insert 101 elements, so one should be missing
	EXPECT_EQ (0, count_other); //Each element was inserted once, so no element should appears more than once or less than zero
}
TEST(ChainedReservoirSampling, Distribution) { 
	ChainedReservoirSampling<int, 100, funct> rs;
	int pre_count[100] = {0};
	int count[100] = {0};
	pre_count[50] = 1000;
	pre_count[25] = 500;
	pre_count[75] = 250;
	unsigned int timestamp = 1;
	for(int i = 0; i < 100; ++i)
		for(int j = 0; j < pre_count[i]; ++j)
			rs.add(i, timestamp++);
	for(int i = 0; i < 100; ++i){
		int idx = rs[i];
		if(idx >= 0 && idx < 100)
			count[idx] += 1;
	}

	EXPECT_TRUE(count[50] > count[25]);
	EXPECT_TRUE(count[25] > count[75]);
	for(int i = 0; i < 100; ++i)
		if(i != 50 && i != 25 && i != 75)
			EXPECT_EQ (0 , count[i]);
}
TEST(ChainedReservoirSampling, Obsolete) { 
	int const size_count = 201;
	int const idx_obsolete = 100;
	int const reservoir_size = 100;
	ChainedReservoirSampling<int, reservoir_size, funct> rs;
	int count[size_count] = {0};
	for(int i = 0; i < size_count; ++i)
		rs.add(i, i);
	rs.obsolete(idx_obsolete); //Desclare timestamps *idx_obsolete* and previous obsolete

	int total_count = 0;
	for(int i = 0; i < idx_obsolete; ++i){//Loop over the sample
		int idx = rs[i];
		if(idx >= 0 && idx < size_count){
			count[idx] += 1;
			total_count += 1;
		}
	}
	EXPECT_EQ(idx_obsolete, total_count);
	for(int i = 0; i < idx_obsolete; ++i) //The first *idx_obsolete* elements should be obsolete, thus not appears.
		EXPECT_EQ (0 , count[i]);
	//We cannot say anything for the other elements
}


