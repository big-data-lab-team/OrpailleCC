#include <cstdlib>
#include <cmath>
#include <iostream>
using namespace std;
#include "gtest/gtest.h"
#include "reservoir_sampling.hpp"

namespace ReservoirSamplingTest{
class functions{
	public:
	static double random(void){
		return static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
	}
	template<class T>
	static int floor(T const x){
		return std::floor(x);
	}
};

template<int size_count>
void test_random_function(double const epsilon, double const delta){
	//Allowable error (epsilon): 0.0O1
	//Probability of error (delta) : 0.00001
	//Range value (R): 1.0
	//iteration = R²ln(1/d) / (2e²) -> 5796462
	int const iteration = 2 * (std::log(1/delta) / (2 * epsilon * epsilon)); //2 time for safety :)
	double count[size_count] = {0};
	for(int j = 0; j < iteration; ++j){
		int const idx = functions::floor(functions::random() * static_cast<double>(size_count));
		assert(idx >= 0 && idx < size_count);
		EXPECT_GE(idx, 0);
		EXPECT_LT(idx, size_count);
		count[idx] += 1;
	}
	for(int i = 0; i < size_count; ++i){
		double const expected_probability = 1.0 / static_cast<double>(size_count);
		double const observed_probability = count[i] /static_cast<double>(iteration);
		EXPECT_NEAR(observed_probability, expected_probability, epsilon);
	}
}
TEST(ReservoirSampling, random) { //Test the random function to make sure it works as expected
	srand(time(NULL));
	double const epsilon = 0.001;
	double const delta = 0.00001;
	test_random_function<10>(epsilon, delta);
	test_random_function<27>(epsilon, delta);
	test_random_function<53>(epsilon, delta);
}
TEST(ReservoirSampling, Add) { 
	ReservoirSampling<int, 100, functions> rs;
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
	ReservoirSampling<int, 100, functions> rs;
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
template<int reservoir_size, int size_count>
void test_reservoir_sampling_statistics(double const epsilon, double const delta){
	//Allowable error (epsilon): 0.0O1
	//Probability of error (delta) : 0.0001
	//Range value (R): 1.0
	//iteration = R²ln(1/d) / (2e²) -> 4605171
	int const iteration = 2 * (std::log(1/delta) / (2 * epsilon * epsilon)); //2 time for safety :)

	double count[size_count] = {0};
	for(int j = 0; j < iteration; ++j){
		ReservoirSampling<int, reservoir_size, functions> ers;
		for(int i = 0; i < size_count; ++i)
			ers.add(i);

		for(int i = 0; i < reservoir_size; ++i){
			count[ers[i]] += 1;
		}
	}
	for(int i = 0; i < size_count; ++i){
		double const expected_probability = static_cast<double>(reservoir_size) / static_cast<double>(size_count);
		double const observed_probability = count[i] /static_cast<double>(iteration);
		EXPECT_NEAR(observed_probability, expected_probability, epsilon);
	}
}
TEST(ReservoirSampling, statistics) { 
	double const epsilon = 0.01;
	double const delta = 0.0001;
	test_reservoir_sampling_statistics<10, 11>(epsilon, delta);
	test_reservoir_sampling_statistics<10, 49>(epsilon, delta);
	test_reservoir_sampling_statistics<10, 27>(epsilon, delta);
	test_reservoir_sampling_statistics<7, 11>(epsilon, delta);
	test_reservoir_sampling_statistics<7, 49>(epsilon, delta);
	test_reservoir_sampling_statistics<7, 27>(epsilon, delta);
	test_reservoir_sampling_statistics<23, 49>(epsilon, delta);
	test_reservoir_sampling_statistics<23, 27>(epsilon, delta);
	test_reservoir_sampling_statistics<7, 103>(epsilon, delta);
	test_reservoir_sampling_statistics<47, 147>(epsilon, delta);
}
}
