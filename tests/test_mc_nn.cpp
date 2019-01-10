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
	/*
	 * In that test case we ensure that when an element is inserted in the classifier it can be predicted well
	 */
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
		auto predict = classifier.predict(dataset + (i*4));
		EXPECT_EQ (label, predict);
	}
}
TEST(MCNN, split) { 
	int dataset_size = 13;
	#define features_count 4
	MCNN<int, features_count> classifier;
	int dataset[dataset_size][features_count];
	int labels[dataset_size];
	for(int i = 0; i < dataset_size; ++i){
		labels[i] = 0;
		for(int j = 0; j < features_count; ++j)
			dataset[i][j] = 0;
	}
	
	//So class 0 will be around [0,0,0,0] and class 1 around [30,0,O,0]
	dataset[1][0] = 30;
	labels[1] = 1;


	for(int idx = 2; idx < 13; ++idx)
		dataset[idx][0] = 20 + (rand()%4); //The new cluster should be around 21 on the first feature

	//Insert the two first instances to initialize the first micro-clusters
	classifier.train(dataset[0], labels[0]);	
	classifier.train(dataset[1], labels[1]);	

	//Two points with different classes were inserted, so we expect 2 clusters
	EXPECT_EQ(2, classifier.count_clusters());

	//The cluster should not be split, so *classifier* should misclassify these datapoints
	for(int idx = 2; idx < 13; ++idx)
		EXPECT_FALSE(labels[idx] == classifier.predict(dataset[idx]));

	//Train with data that belongs to the class 0 but that are closer to class 1
	for(int idx = 2; idx < 13; ++idx)
		classifier.train(dataset[idx], labels[idx]);	
	EXPECT_TRUE(classifier.count_clusters() > 2);
	//We expect the classifier to have split at least one cluster and to be able to predict the data around 21 on the first feature
	for(int idx = 0; idx < 13; ++idx)
		EXPECT_EQ(labels[idx], classifier.predict(dataset[idx]));
}
