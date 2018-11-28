#include <ctime>
#include "gtest/gtest.h"
#include "mc_nn.hpp"

TEST(MCNN, one) { 
	MCNN<int, 4> mcnn();
	srand(time(NULL));
	MCNN<int, 4> classifier;
	int* dataset = (int*)calloc(4 , sizeof(int));
	int label = rand() % 100;
	classifier.train(dataset, label);	
	auto predict = classifier.predict(dataset);
	EXPECT_EQ (label, predict);
}

TEST(MCNN, insert_once) { 
	MCNN<int, 4> mcnn();
	srand(time(NULL) - 10);
	MCNN<int, 4> classifier;
	int dataset_size = 4;
	int* dataset = (int*)calloc(4 * dataset_size , sizeof(int));
	int labels[dataset_size];
	for(int i = 0; i < dataset_size; ++i){
		for(int j = 0; j < 4; ++j)
			dataset[i * 4 + j] = rand() % 100;
		labels[i] = i + 2;
		classifier.train(dataset + (i*4), labels[i]);	
	}
	for(int i = 0; i < dataset_size; ++i){
		auto predict = classifier.predict(dataset + (i*4));
		EXPECT_EQ (labels[i], predict);
	}
}
TEST(MCNN, same_class) { 
	MCNN<int, 4> mcnn();
	srand(time(NULL) + 30);
	MCNN<int, 4> classifier;
	int dataset_size = 4;
	int* dataset = (int*)calloc(4 * dataset_size , sizeof(int));
	int label = rand()%100;
	for(int i = 0; i < dataset_size; ++i){
		for(int j = 0; j < 4; ++j)
			dataset[i * 4 + j] = rand() % 100;
		classifier.train(dataset + (i*4), label);	
	}
	for(int i = 0; i < dataset_size; ++i){
		auto predict = classifier.predict(dataset);
		EXPECT_EQ (label, predict);
	}
}
