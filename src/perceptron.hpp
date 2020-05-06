/**
 * Implement a Multi-Layers Perceptron object. The network is divided by layers and each layer of perceptron is fully connected to the previous one.
 * - layer_count: the number of layer.
 * - max_size: the maximal size used by the object. Note that this size if set at the beginning.
 * - func: a class type that contains all needed function for the MultiLayerPerceptron.
 *   	+ random function: A function that returns a random number between 0 and 1, to initialize the weights.
 *   	+ activation function: A function that act as the activation function.
 *   	+ derivative function: The derivative of the activation function.
 */
template<int layer_count, int max_size, class func>
class MultiLayerPerceptron{
	static const int total_weight_count = ((max_size + sizeof(double) - max_size%sizeof(double)) / sizeof(double))/2;
	double learning_rate = 0.1;
	//Contains the weights of the network.
	double weights[total_weight_count];
	//Contain the last output of each neuron or the last backpropagation error depending on the last function called.
	double neuron_output[total_weight_count];

	//The size of each layer.
	int layer_size[layer_count];

	/**
	 * Return the the base index of the of the layer layer_idx in the array weights.
	 * @param layer_idx The index of the layer targeted. It should start at 1 since the layer 0 doesn't have weights (it's the input layer).
	 */
	int get_weight_base(int const layer_idx) const{
		int layer_base = 0;
		for(int i = 1; i < layer_idx; ++i)
			layer_base += (layer_size[i-1]+1) * layer_size[i];
		return layer_base;
	}
	/**
	 * Return the the base index of the of the layer layer_idx in the array neuron_output.
	 * @param layer_idx The index of the layer targeted. It should start at 0 since it is the input layer.
	 */
	int get_output_base(int const layer_idx) const{
		int layer_base = 0;
		for(int i = 0; i < layer_idx; ++i)
			layer_base += layer_size[i];
		return layer_base;
	}
	public:
		/**
		 * Constructor that take the shape of the neural network, thus the size of each layer.
		 * @param layer_size An array of size layer_count that contains for each index the size of the layer.
		 * @param learning_rate The learning rate of the network.
		 */
		MultiLayerPerceptron(int const* layer_size, double const learning_rate=0.1){
			this->learning_rate = learning_rate;
			for(int i = 0; i < layer_count; ++i){
				this->layer_size[i] = layer_size[i];
			}
			//Randomly initialize the weights
			for(int i = 0; i < total_weight_count; ++i)
				weights[i] = func::random();
		}
		/**
		 * Pass the input trough the neural network.
		 * @param input The input value for the first layer of the neural network. Should be in the size of the first layer
		 * @param output The result of the last layer. Should be in the size of the last layer.
		 */
		void feed_forward(double const* input, double* output){
			//Initialize input neuron
			for(int neuron_idx = 0; neuron_idx < layer_size[0]; ++neuron_idx){
				neuron_output[neuron_idx] = input[neuron_idx];
			}

			//Initialize the base index used at each layer
			int output_base = 0;
			int weight_base = 0;

			//Feed forward for each layer
			for(int layer_idx = 1; layer_idx < layer_count; ++layer_idx){
				//For each neuron, we do the linear combination of the previous layer (including the bias)
				for(int neuron_idx = 0; neuron_idx < layer_size[layer_idx]; ++neuron_idx){ 

					double sum = 0;
					for(int weight_idx = 0; weight_idx < layer_size[layer_idx-1]; ++weight_idx){
						//Compute the index in *weights* and the index in *neuron_output* in the previous layer
						int weight_global_idx = weight_base + neuron_idx * (layer_size[layer_idx-1]+1) + weight_idx; //+1 because of the bias neuron
						int input_global_idx = output_base + weight_idx;
						sum += weights[weight_global_idx] * neuron_output[input_global_idx];
					}
					//Grab the bias neuron
					int weight_global_idx = weight_base + neuron_idx * (layer_size[layer_idx-1]+1) + layer_size[layer_idx-1];
					sum += weights[weight_global_idx];

					//Compute the index of the current neuron in *neuron_output* to store its result
					int output_global_index = output_base + layer_size[layer_idx-1] + neuron_idx;
					neuron_output[output_global_index] = func::activation(sum);	

				}
				weight_base += layer_size[layer_idx] * (layer_size[layer_idx-1]+1); //+1 because of the bias neuron
				output_base += layer_size[layer_idx-1]; //There is not +1 for the neuron output because the bias is alway 1 and never stored
			}

			//Write the result of the last layer in *output*
			for(int neuron_idx = 0; neuron_idx < layer_size[layer_count-1]; ++neuron_idx){
				output[neuron_idx] = neuron_output[output_base + neuron_idx];
			}
		}
		/**
		 * Set the input weights of one neuron.
		 * @param layer_idx The layer where the neuron is. Must be greater or equal to 1, because the first layer do not have weights.
		 * @param neuron_idx The index of the neuron in this layer.
		 * @param new_weights An array in the size of the previous layer plus one that contains the new weights. The last weight is for the bias.
		 */
		void set_weights(int layer_idx, int neuron_idx, double* new_weights){
			int base_index = 0;
			for(int l = 1; l < layer_idx; ++l)
				base_index += layer_size[l] * (layer_size[l-1]+1);
			
			base_index += (layer_size[layer_idx-1]+1) * neuron_idx;
			for(int i = 0; i < layer_size[layer_idx-1]+1; ++i)
				weights[base_index + i] = new_weights[i];
		}
		/**
		 * Set the input weights of one layer.
		 * @param layer_idx The layer where the neuron is. Must be greater or equal to 1, because the first layer do not have weights.
		 * @param new_weights An array in the size of ((previous_layer_size+1) * current_layer) that contains the new weights. 
		 */
		void set_weights(int layer_idx, double* new_weights){
			int base_index = 0;
			for(int l = 1; l < layer_idx; ++l)
				base_index += layer_size[l] * (layer_size[l-1]+1);
			
			for(int i = 0; i < (layer_size[layer_idx-1]+1) * layer_size[layer_idx]; ++i)
				weights[base_index + i] = new_weights[i];
		}
		/**
		 * Set the input weights of the network.
		 * @param new_weights An array that contains the new weights. 
		 */
		void set_weights(double* new_weights){
			int base_index = 0;
			for(int l = 1; l < layer_count; ++l){
				for(int i = 0; i < (layer_size[l-1]+1) * layer_size[l]; ++i)
					weights[base_index + i] = new_weights[base_index + i];
				base_index += (layer_size[l-1]+1) * layer_size[l];
			}
		}
		/**
		 * Return the array of weights.
		 */
		double* const get_weights(void){
			return static_cast<double* const>(static_cast<void*>(weights));
		}
		/**
		 * Run the backpropagation step based on the expected value and the last call to feed_forward.
		 * @param expected .
		 */
		double backpropagate(double const* expected){
			int output_base = get_output_base(layer_count-1);
			int weight_base = get_weight_base(layer_count-1);

			double sum_error_output = 0;
			//Set the error of the last layer
			for(int i = 0; i < layer_size[layer_count-1]; ++i){
				int output_global_index = output_base + i;
				double output = neuron_output[output_global_index];
				double err = output - expected[i];
				sum_error_output += (err*err);
				neuron_output[output_global_index] = func::derivative(output) * err; 
			}

			for(int layer_idx = layer_count - 2; layer_idx >= 0; --layer_idx){
				output_base -= layer_size[layer_idx];
				weight_base -= (layer_size[layer_idx-1]+1) * layer_size[layer_idx];
				for(int neuron_idx = 0; neuron_idx <= layer_size[layer_idx]; ++neuron_idx){
					double neuron_error = 0;
					double current_neuron_output = 1;
					if(neuron_idx < layer_size[layer_idx])
						current_neuron_output = neuron_output[output_base + neuron_idx];
					for(int weight_idx = 0; weight_idx < layer_size[layer_idx+1]; ++weight_idx){

						int neuron_next_layer = output_base + layer_size[layer_idx] + weight_idx;
						int weight_next_layer = weight_base + (layer_size[layer_idx-1]+1) * layer_size[layer_idx] + weight_idx * (layer_size[layer_idx]+1) + neuron_idx;
						neuron_error += neuron_output[neuron_next_layer] * weights[weight_next_layer];
					}
					neuron_error *= func::derivative(current_neuron_output);

					//Second loop to adjust the weights
					for(int weight_idx = 0; weight_idx < layer_size[layer_idx+1]; ++weight_idx){

						int neuron_next_layer = output_base + layer_size[layer_idx] + weight_idx;
						int weight_next_layer = weight_base + (layer_size[layer_idx-1]+1) * layer_size[layer_idx] + weight_idx * (layer_size[layer_idx]+1) + neuron_idx;
						double d_weight = - learning_rate * current_neuron_output * neuron_output[neuron_next_layer];
						weights[weight_next_layer] += d_weight;
					}
					if(neuron_idx < layer_size[layer_idx])
						neuron_output[output_base + neuron_idx] = neuron_error;
				}
			}

			return (sum_error_output/layer_size[layer_count-1]);
		}
};

