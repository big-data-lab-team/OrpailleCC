#include <ctime>
#include "gtest/gtest.h"
#include "knn.hpp"
namespace KnnTest{
class functions{
	public:
	template<class T>
	static int floor(T const x){
		return 1;
	}
	static double random(void){
		return (static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX));
	}
	static double distance(double const* d1, double const* d2, int const feature_count) {
		double ret = 0;
		for(int i = 0; i < feature_count; ++i)
			ret += (d1[i] - d2[i])*(d1[i] - d2[i]);
		return ret;
	}
};
TEST(KNN, one) { 
	srand(time(NULL));
	KNN<double, 2, 5, functions> classifier(3);
	double* dataset = (double*)calloc(4 , sizeof(int));
	int label = rand() % 100;
	classifier.train(dataset, label);	
	auto predict = classifier.predict(dataset);
	EXPECT_EQ (label, predict);
	free(dataset);
}
TEST(KNN, same_class) { 
	srand(time(NULL) + 30);
	KNN<double, 2, 5, functions> classifier(3);
	int dataset_size = 4;
	double* dataset = (double*)calloc(4 * dataset_size , sizeof(double));
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
	free(dataset);
}
TEST(KNN, closest) { 
	srand(time(NULL) + 30);
	KNN<double, 2, 5, functions> classifier(4);
	int const dataset_size = 6;
	double* dataset = (double*)calloc(2 * dataset_size , sizeof(double));
	dataset[0] = 0;
	dataset[1] = 0;
	dataset[2] = 1;
	dataset[3] = 1;
	dataset[4] = 1;
	dataset[5] = 0;
	dataset[6] = 5;
	dataset[7] = 5;
	dataset[8] = 6;
	dataset[9] = 5;
	dataset[10] = 6;
	dataset[11] = 6;
	classifier.train(dataset, 0);
	classifier.train(dataset+2, 0);
	classifier.train(dataset+4, 0);
	classifier.train(dataset+6, 1);
	classifier.train(dataset+8, 1);
	classifier.train(dataset+10, 1);

	double test1[2] = {0.5, 0.5};
	double test2[2] = {5.5, 5.5};
	EXPECT_EQ (0, classifier.predict(test1));
	EXPECT_EQ (1, classifier.predict(test2));
	free(dataset);
}
}
