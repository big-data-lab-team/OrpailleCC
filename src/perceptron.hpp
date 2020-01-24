template<int layer_count, int max_size, class funct>
class MultiLayerPerceptron{
	static const int total_weight_count = ((max_size + sizeof(double) - max_size%sizeof(double)) / sizeof(double))/2;
	double weights[total_weight_count];
	double neuron_output[total_weight_count];

	int layer_size[layer_count];

	public:
		MultiLayerPerceptron(int* layer_size){
			for(int i = 0; i < layer_count; ++i){
				this->layer_size[i] = layer_size[i];
			}
		}
		/**
		 * Pass the input trough the neural network.
		 * @param input The input value for the first layer of the neural network. Should be in the size of the first layer
		 * @param output The result of the last layer. Should be in the size of the last layer.
		 */
		void feed_forward(double* input, double* output){
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
					neuron_output[output_global_index] = funct::activation(sum);	

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
};

