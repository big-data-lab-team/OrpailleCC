#include <cstdlib>
#include <cmath>
#include <iostream>
using namespace std;
#include "gtest/gtest.h"
#include "perceptron.hpp"

namespace PerceptronTest{
class functions{
	public:
	static double activation(double input){
		return input;
	}
	template<class T>
	static int derivative(T const x){
		return 1;
	}
	static double random(void){
		return (static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX));
	}
};

TEST(MultilayerPerceptron, propagating) {
	int layer_size[3] = {2, 2, 1};
	double weights[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};//There is 9 because of the bias neuron at each layer
	double input[2] = {1.5, 3}, output[2] = {0}, expected[1] = {12};
	MultiLayerPerceptron<3, 60000, functions> mlp(layer_size);
	mlp.set_weights(weights);
	mlp.feed_forward(input, output);

	EXPECT_NEAR(output[0], expected[0], 0.0001);
}
TEST(MultilayerPerceptron, propagating2) {
	int layer_size[3] = {2, 2, 1};
	double initial_weights[9] = {1,1,1,1,1,1,1,1,1}; //There is 9 because of the bias neuron at each layer
	double weights[6] = {0.5, 0.5, 0.0, 0.25, 0.25, 0.0};
	double input[2] = {1.5, 3}, output[2] = {0}, expected[1] = {4.375};
	MultiLayerPerceptron<3, 60000, functions> mlp(layer_size);
	mlp.set_weights(initial_weights);
	mlp.set_weights(1, 0, weights);
	mlp.set_weights(1, 1, weights+3);
	mlp.feed_forward(input, output);

	EXPECT_NEAR(output[0], expected[0], 0.0001);
}
TEST(MultilayerPerceptron, backpropagate) {
	int layer_size[3] = {2, 2, 1};
	double initial_weights[9] = {1,1,1,1,1,1,1,1,1}; //There is 9 because of the bias neuron at each layer
	double weights[6] = {0.5, 0.5, 0.1, 0.25, 0.25, 0.1};
	double input[2] = {1.5, 3}, output[2] = {0}, expected[1] = {2.849713370};
	double real_output[1] = {4.0};
	MultiLayerPerceptron<3, 60000, functions> mlp(layer_size);
	mlp.set_weights(initial_weights);
	mlp.set_weights(1, 0, weights);
	mlp.set_weights(1, 1, weights+3);
	mlp.feed_forward(input, output);
	mlp.backpropagate(real_output);
	mlp.feed_forward(input, output);

	EXPECT_NEAR(output[0], expected[0], 0.0001);
}
}

