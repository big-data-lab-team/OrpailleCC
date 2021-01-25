#include <stdio.h>
#include <stdlib.h>
#include <typeinfo>
#include "bloom_filter.hpp"
#include "cuckoo_filter.hpp"
#include "reservoir_sampling.hpp"
#include "mondrian_coarse.hpp"

//#ifdef DEBUG
//#include <iostream>
//using namespace std;
//#endif


struct funct{
 //*   	+ exp function: A function that compute the exponential of a double. (Used to compute the posterior means)
 //*   	+ rand_uniform function: A function that pick uniformly a random number between [0,1[.
 //*   	+ log function: A function that run the natural logarithm.
	static double exp(double const x){
		return 0;
	}
	static double rand_uniform(void){
		return 0;
	}
	static double log(double const x){
		return 0;
	}
};
#define COUNT_ENTRY 14
#define FEATURE_COUNT 4
#define LABEL_COUNT 2
double dt[COUNT_ENTRY][FEATURE_COUNT] =  {{2, 2, 1, 0},
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
int labels[COUNT_ENTRY] = {0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0};
template<int feature_count, int label_count>
class ErrorMetrics{
	int count = 0;
	int error_count = 0;
	public:
	void update(int const true_label, int const prediction){
		count += 1;
		error_count += (true_label != prediction);
	}
	double score(void) const{
		return static_cast<double>(error_count) / static_cast<double>(count);
	}
	void increase_error(int const c=1){
		count += c;
		error_count += c;

	}
};
int main(){

	CoarseMondrianForest<double, funct, ErrorMetrics<FEATURE_COUNT, LABEL_COUNT>, FEATURE_COUNT, LABEL_COUNT, 6000> classifier(0.6, 0.0001, 0.0, 10);
	classifier.train(dt[0], labels[0]);
	classifier.train(dt[2], labels[2]);

	double scores[2];
	int result = classifier.predict(dt[0], scores);
	cout << "Prediction: " << result << endl;
	cout << "True Label: " << labels[0] << endl;
	//result = classifier.predict(dt[2], scores);
}
