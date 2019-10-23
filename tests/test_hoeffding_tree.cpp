#include <ctime>
#include <cmath>
#include <algorithm>
#include "gtest/gtest.h"
#include "hoeffding_tree.hpp"

#define COUNT_ENTRY_HT 14
#define FEATURE_COUNT_HT 2
namespace HoeffdingTreeTest{
double dt[COUNT_ENTRY_HT][FEATURE_COUNT_HT] =  {{2, 0},
												{2, 1},
												{1, 0},
												{0, 0},
												{0, 0},
												{0, 1},
												{1, 1},
												{2, 0},
												{2, 0},
												{0, 0},
												{2, 1},
												{1, 1},
												{1, 0},
												{0, 1}
												};
double labels[COUNT_ENTRY_HT] = {0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0};
class functions{
	public:
	static double log(double const x){
		return std::log(x);
	}
	static double log2(double const x){
		return std::log2(x);
	}
	static double sqrt(double const x){
		return std::sqrt(x);
	}
	static bool isnan(double const x){
		return std::isnan(x);
	}
};

TEST(HoeffdingTree, compute_information_gain) { 
	int features_size[2] = {3, 2};
	HoeffdingTree<double, 2, 2, 10000, functions> ht(0.99, features_size);
	auto root = ht.get_root();

	double new_limits[3] = {0.5, 1.5, 0.5};
	root->set_limits(new_limits);
	for(int i = 0; i < COUNT_ENTRY_HT; ++i){
		double split_value;
		root->train(&dt[i][0], labels[i], split_value);
		//Just ignore the split return
	}

	double info_gain[3];
	root->compute_information_gain(info_gain);
	std::sort(info_gain, info_gain+3);	
	ASSERT_NEAR(info_gain[0], 0.003184, 0.0001);
	ASSERT_NEAR(info_gain[1], 0.048127, 0.0001);
	ASSERT_NEAR(info_gain[2], 0.102243, 0.0001);
}
TEST(HoeffdingTree, is_leaf) { 
	int const features_size[2] = {3, 2};
	HoeffdingTree<double, 2, 2, 10000, functions> ht(0.5, features_size);
	auto root = ht.get_root();
	
	EXPECT_EQ (true, root->is_leaf());
}
TEST(HoeffdingTree, is_leaf2) { 
	int features_size[2] = {3, 2};
	HoeffdingTree<double, 2, 2, 10000, functions> ht(0.3, features_size);
	auto root = ht.get_root();

	double new_limits[3] = {0.5, 1.5, 0.5};
	root->set_limits(new_limits);
	for(int i = 0; i < 100*COUNT_ENTRY_HT; ++i){ //200 hundred data points should be enough to produce a split with this dataset
		double split_value;
		root->train(&dt[i%COUNT_ENTRY_HT][0], labels[i%COUNT_ENTRY_HT], split_value);
	}
	ht.train(&dt[0][0], labels[0]);
	EXPECT_EQ (false, root->is_leaf()); //A split should have occured so root should not be a leaf anymore
}
}
