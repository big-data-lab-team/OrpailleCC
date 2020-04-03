#include "gtest/gtest.h"
#include <cstdio>
#include <cmath>
#include <iostream>
using namespace std;
#include "naive_bayes.hpp"

#define COUNT_ENTRY_NB 14
#define FEATURE_COUNT_NB 4
double dt[COUNT_ENTRY_NB][FEATURE_COUNT_NB] =  {{2, 2, 1, 0},
												{2, 2, 1, 1},
												{1, 2, 1, 0},
												{0, 1, 1, 0},
												{0, 0, 0, 0},
												{0, 0, 0, 1},
												{1, 0, 0, 1},
												{2, 1, 1, 0},
												{2, 0, 0, 0},
												{0, 1, 0, 0},
												{2, 1, 0, 1},
												{1, 1, 1, 1},
												{1, 2, 1, 0},
												{0, 1, 1, 1}
												};
int labels[COUNT_ENTRY_NB] = {0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0};

class functions{
	public:
	static double log(double const x){
		return std::log(x);
	}
	static double sqrt(double const x){
		return std::sqrt(x);
	}
};
TEST(NaiveBayes, basic) { 
	NaiveBayes<double, 2,FEATURE_COUNT_NB,functions> classifier;
	classifier.train(dt[0], labels[0]);

	double scores[2];
	int result = classifier.predict(dt[0], scores);
	EXPECT_EQ (result, labels[0]);
}
TEST(NaiveBayes, basic2) { 
	NaiveBayes<double, 2,FEATURE_COUNT_NB,functions> classifier;
	classifier.train(dt[0], labels[0]);
	classifier.train(dt[2], labels[2]);

	double scores[2];
	int result = classifier.predict(dt[0], scores);
	EXPECT_EQ (result, labels[0]);
	result = classifier.predict(dt[2], scores);
	EXPECT_EQ (result, labels[2]);
}
TEST(NaiveBayes, prediction) { 
	NaiveBayes<double, 2,FEATURE_COUNT_NB,functions> classifier;
	for(int i = 0; i < COUNT_ENTRY_NB; ++i)
		classifier.train(dt[i], labels[i]);

	double scores[2];
	double haha[FEATURE_COUNT_NB] = {2, 0, 1, 1};
	int result = classifier.predict(haha, scores);
	EXPECT_EQ (0, result);
}
//TEST(NaiveBayes, smoothing) { 
	//int features_size[FEATURE_COUNT_NB] = {3, 3, 2, 2};
	//NaiveBayes<2,FEATURE_COUNT_NB,functions> classifier(features_size);
	//for(int i = 0; i < COUNT_ENTRY_NB; ++i)
		//classifier.train(dt[i], labels[i]);

	//double scores[2];
	//int haha[FEATURE_COUNT_NB] = {2, 0, 1, 1};
	//int result = classifier.predict(haha, scores);
	//EXPECT_EQ (0, result);

	//double smoothing = classifier.get_smoothing();
	//double scores_sm[2];
	//classifier.set_smoothing(0.1);
	//result = classifier.predict(haha, scores_sm);
	//ASSERT_NE(scores_sm[0], scores[0]);		
	//ASSERT_NE(scores_sm[1], scores[1]);		
//}
