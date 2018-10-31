#include <cstdlib>
#include "gtest/gtest.h"
#include "reservoir_sampling.hpp"
struct random_lambda{
	double operator()(void){
		return (double)rand() / (double)RAND_MAX;
	}
};
TEST(ReservoirSampling, Add) { 
	ReservoirSampling<int, 100, random_lambda> rs;
	int count[100] = {0};
	for(int i = 0; i < 100; ++i)
		rs.add(i);
	for(int i = 0; i < 100; ++i){
		int idx = rs[i];
		if(idx >= 0 && idx < 100)
			count[idx] += 1;
	}
	for(int i = 0; i < 100; ++i)
		EXPECT_EQ (1 , count[i]);
}

TEST(ReservoirSampling, Distribution) { 
	ReservoirSampling<int, 100, random_lambda> rs;
	int pre_count[100] = {0};
	int count[100] = {0};
	pre_count[50] = 1000;
	pre_count[25] = 500;
	pre_count[75] = 250;
	for(int i = 0; i < 100; ++i)
		for(int j = 0; j < pre_count[i]; ++j)
			rs.add(i);
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
