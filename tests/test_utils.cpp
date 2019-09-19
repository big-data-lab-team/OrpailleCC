#include <stdlib.h>
#include <cmath>
#include "gtest/gtest.h"
#include "utils.hpp"

class functions{
	public:
	static double rand_uniform(void){
		return static_cast<double>(rand())/static_cast<double>(RAND_MAX);
	}
	static double log(double const x){
		return std::log(x);
	}
};

TEST(Utils, turn_array_into_probability) { 
	double values[3] = {6, 15, 9};
	Utils::turn_array_into_probability(values, 3);
	ASSERT_DOUBLE_EQ(values[0], 0.2);
	ASSERT_DOUBLE_EQ(values[1], 0.7);
	ASSERT_DOUBLE_EQ(values[2], 1.0);
}
TEST(Utils, turn_array_into_probability2) { 
	double values[3] = {6, 15, 9};
	Utils::turn_array_into_probability(values, 3, 30);
	ASSERT_DOUBLE_EQ(values[0], 0.2);
	ASSERT_DOUBLE_EQ(values[1], 0.7);
	ASSERT_DOUBLE_EQ(values[2], 1.0);
}
TEST(Utils, turn_array_into_probability3) { 
	double values[4] = {10, 10, 10, 10};
	Utils::turn_array_into_probability(values, 4, 40);
	ASSERT_DOUBLE_EQ(values[0], 0.25);
	ASSERT_DOUBLE_EQ(values[1], 0.5);
	ASSERT_DOUBLE_EQ(values[2], 0.75);
	ASSERT_DOUBLE_EQ(values[3], 1.0);
}
TEST(Utils, pick_from_distribution) { 
	//Probability of error (d) : 0.0001
	//Allowable error (e): 0.01
	//Range value (R): 1.0
	//total_count = R²ln(1/d) / (2e²) -> 46052
	//total_count rounded a bit up.
	double distribution[3] = {0.2, 0.7, 1.0};
	double expected[3] = {0.2, 0.5, 0.3};
	double counters[3] = {0};
	int const total_count = 48000;
	for(int i = 0; i < total_count; ++i){
		int e = Utils::pick_from_distribution<functions>(distribution, 3);
		EXPECT_TRUE(e >= 0 && e < 3);
		counters[e] += 1;
	}
	for(int i = 0; i < 3; ++i){
		counters[i] /= static_cast<double>(total_count);
		ASSERT_NEAR(counters[i], expected[i], 0.01);
	}
}
TEST(Utils, rand_exponential) { 
	double sum = 0;
	int const total_count = 48000;
	double const rate = 27;
	double const expected = 1.0/27.0;
	for(int i = 0; i < total_count; ++i){
		sum += Utils::rand_exponential<functions>(rate);
	}
	double const mean = sum / static_cast<double>(total_count);
	ASSERT_NEAR(mean, expected, 0.01);
}

