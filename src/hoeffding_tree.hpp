/**
 * Implement the Hoeffding Tree algorithm.
 * - feature_type: the type of the feature (int, double, float, short, ...).
 * - feature_count: the number of feature per data point.
 * - label_count: the number of label.
 * - max_size: the maximum value the data point can reach.
 * - func: a class type that contains all needed function for the Hoeffding Tree.
 *   	+ log function: A function that run the natural logarithm.
 *   	+ log2 function: A function that run the base two logarithm.
 *   	+ isnan function: A function that run true if the first parameter is Not a Number.
 */

template<class feature_type, int feature_count, int label_count, int max_size, class func>
class HoeffdingTree{
	static int const EMPTY_NODE = -1;
	//The buffer that will store all the node and the counters
	char buffer[max_size] = {0};
	//Offsets from the beginning and the end of the buffer
	int buffer_offset = 0, buffer_preset = max_size;
	double delta; //Probability of error in the Hoeffding bound
	//The number of values for each feature
	int features_size[feature_count];
	//The sum of the value in features_size
	int sum_feature_size = 0;

	public:
	class Node{
		//Declare HoeffdingTree friend to its own nodes because Node may be exposed throught a public interface.
		friend class HoeffdingTree;
		//A pointer to the tree the node belongs
		HoeffdingTree* tree;
		//The spliting feature if it is an internal node, otherwise, it is the location of the counters in buffer
		int split_feature;
		//The value to use for the split.
		double split_value;
		//The upper and lower bounds of the data points encountered in this node. Note that parent's *split_value* may influence these bounds
		double l_box[feature_count];
		double u_box[feature_count];
		//The children of the node if it is an internal node
		int children[2]; //children[0] == left, children[1] == right
		public:
		/**
		 * The constructor of the node.
		 * @param t The tree that contains the node.
		 */
		Node(HoeffdingTree& t){
			this->tree = &t;
			// find the index for the counters :>
			// NOTE: in the new operator, we made sure that there was space for the node and its counters.
			int const total_size_counters = compute_total_size_counters(tree->sum_feature_size);
			int const tmp_preset = tree->buffer_preset - total_size_counters;
			this->split_feature = - ((max_size - tmp_preset) / total_size_counters); //we need negative number because counters are store on the other side of buffer
			reset_counters();
			double const infinity = std::numeric_limits<float>::infinity();
			for(int i = 0; i < feature_count; ++i){
				l_box[i] = -infinity;
				u_box[i] =  infinity;
			}
			int* count;
			double* limits;
			double* info_sum;
			get_informations(count, limits, info_sum);
			children[0] = EMPTY_NODE;
			children[1] = EMPTY_NODE;
		}
		/**
		 * The constructor of the node using another node.
		 * The new node will inherit of the counters' memory, the tree pointer and the box to help decide the split values.
		 * @param parent The node from which attributes will be taken.
		 */
		Node(Node const& parent){
			this->split_feature = parent.split_feature;	
			this->tree = parent.tree;
			reset_counters();
			for(int i = 0; i < feature_count; ++i){
				l_box[i] = parent.l_box[i];
				u_box[i] = parent.u_box[i];
			}
			children[0] = EMPTY_NODE;
			children[1] = EMPTY_NODE;
		}
		/**
		 * Return true if the node is a leaf.
		 */
		bool is_leaf(void) const{
			return (split_feature < 0);
		}
		/**
		 * Predict the class of the data point *features*.
		 * @param features The value of the data point.
		 * @param probabilities The output array that will contains the score for each label.
		 */
		int predict(feature_type const* features, double* probabilities) const{
			int* count;
			double* limits;
			double* info_sum;
			int* counters[label_count];
			for(int i = 0; i < label_count; ++i)
				counters[i] = get_counters(i);
			get_informations(count, limits, info_sum);

			//Computing the majority vote
			double counts[label_count] = {0};
			double sum = 0;
			for(int l = 0; l < label_count; ++l){ //we want the number of occurence per label
				for(int i = 0; i < tree->features_size[0]; ++i) //this number of occurence is the sum of occurence for all value of the first feature ... or the second ...
					counts[l] += counters[l][i];
				sum += counts[l];
			}
			int best = 0;
			//Compute the probabilities.
			//NOTE: *l* starts at 0 because if it starts at 1, the first probability won't be computed and stored in the array
			for(int l = 0; l < label_count; ++l){
				probabilities[l] = counts[l] / sum;
				if(probabilities[l] > probabilities[best])
					best = l;
			}
			return best;
		}
		/**
		 * Split the current node in two on the *feature* with the spliting value of *value*.
		 * This function return 0 if everything is alright.
		 * @param feature The feature for the split.
		 * @param value The value to use for the split.
		 */
		int split(int const feature, double const value){
			Node* child_right = new (*tree) Node(*tree);
			//NOTE: child_left has been created re-using counters and bounds from current node
			Node* child_left = new (*tree) Node(*this); 
			if(child_left == nullptr || child_right == nullptr) // No more node available
				return -1;
			//This operation return the index of child_left since the cell size of the pointer is sizeof Node
			children[0] = child_left - reinterpret_cast<Node*>(tree->buffer); 
			children[1] = child_right - reinterpret_cast<Node*>(tree->buffer);

			//Set the value of split for this node. From here we don't have access to the previous counters.
			split_feature = feature;
			split_value = value;

			//NOTE: *child_left* has been use the box of its parent already
			//Just set the box for *child_right*
			for(int i = 0; i < feature_count; ++i){
				child_right->u_box[i] = u_box[i];
				child_right->l_box[i] = l_box[i];
			}
			//Make the box of the children a bit more accurate before they start looking for split values
			child_left->u_box[split_feature] = split_value;
			child_right->l_box[split_feature] = split_value;

			//Select the values.
			child_right->select_split_values();
			child_left->select_split_values();
			return 0;
		}
		/**
		 * Train the tree with a data point.
		 * @param features The data point.
		 * @param label The label of the data point.
		 * @param An output value. Will contain the value suggested for the split if the return value is not negative.
		 */
		int train(feature_type const* features, int const label, double& split_value){
			int* count;
			double* limits;
			double* info_sum;
			int* counters[label_count];
			for(int i = 0; i < label_count; ++i)
				counters[i] = get_counters(i);
			get_informations(count, limits, info_sum);

			//We increment the counters to train the node
			int tmp_feature_sum = 0;
			int* label_counters = counters[label];
			
			//There one more data point in this leaf
			*count += 1;
				 
			//Update the box
			for(int f = 0; f < feature_count; ++f){
				u_box[f] = u_box[f] < features[f] ? features[f] : u_box[f];
				l_box[f] = l_box[f] > features[f] ? features[f] : l_box[f];
			}

			for(int f = 0; f < feature_count; ++f){
				bool limit_found = false;
				//Look for the bin
				for(int lim_index = 0; lim_index < (tree->features_size[f]-1); ++lim_index){ //There is one limit less than there  is bins
					//NOTE the limits are supposed to be sorted
					if(features[f] < limits[tmp_feature_sum+lim_index-f]){
						label_counters[tmp_feature_sum + lim_index] += 1;
						limit_found = true;
						break;
					}
				}

				if(!limit_found) //We didn't found the limit in the loop, then it is the last bin
					label_counters[tmp_feature_sum +(tree->features_size[f]-1)] += 1;
				tmp_feature_sum += tree->features_size[f];
			}
			//Now we need to update *info_sum*
			double info_gain[tree->sum_feature_size-feature_count];			
			//NOTE: no need to initialize output because it will be set.
			compute_information_gain(static_cast<double>(*count), counters, info_gain);
			double const d_count = static_cast<double>(*count);
			for(int i = 0; i < (tree->sum_feature_size-feature_count); ++i){
				info_sum[i] += info_gain[i];
				info_gain[i] = info_sum[i] / d_count;
			}

			//Look for the two best split.
			//*best_split* contains indexes.
			int best_split[2] = {0, 1};
			//The information gain of best_split[0] is always greater than best_split[1]
			//So we may need to switch it
			if(info_gain[best_split[0]] < info_gain[best_split[1]]){
				best_split[0] = 1;
				best_split[1] = 0;
			}
			//Browse indexes higher than 1 and keep the order of *best_split* when replacing an index
			for(int i = 2; i < (tree->sum_feature_size-feature_count); ++i){
				if(info_gain[i] > info_gain[best_split[0]]){
					best_split[1] = best_split[0];
					best_split[0] = i;
				}
				else if(info_gain[i] > info_gain[best_split[1]]){
					best_split[1] = i;
				}
			}
			//compute Hoeffding bound
			double const epsilon = 4 * func::log(1/tree->delta) / (2*d_count);
			double const diff = (info_gain[best_split[0]] - info_gain[best_split[1]]);
			double const square_diff = diff * diff;
			if(square_diff > epsilon && info_gain[best_split[0]] > 0){
				split_value = limits[best_split[0]];
				//Try to retrieve the feature concerned :>
				int tmp_c = tree->features_size[0];
				for(int f = 0; f < feature_count-1; ++f){
					if(best_split[0] < tmp_c)
						return f;
					tmp_c += tree->features_size[f+1];
				}
				//If the feature is not found, it must be the last one
				return feature_count-1;
			}
			return -1;
		}
		/**
		 * Compute the information gain for each split considered by the node.
		 * The information gain are store in *output*.
		 * @param output An array of size *sum_feature_size-feature_count* where *sum_feature_size* is the sum of the number of value per feature given to the constructor of the tree.
		 */
		void compute_information_gain(double* output) const{
			int* count;
			double* limits;
			double* info_sum;
			int* counters[label_count];
			for(int i = 0; i < label_count; ++i)
				counters[i] = get_counters(i);
			get_informations(count, limits, info_sum);
			compute_information_gain(static_cast<double>(*count), counters, output);
		}
		/**
		 * Compute information gain using given counters.
		 * The information gain are store in *output*.
		 * @param count_data_points The total number of data points in that node.
		 * @param counters The counter for each label, each feature and each bin in these feature.
		 * @param output An array of size *sum_feature_size-feature_count* where *sum_feature_size* is the sum of the number of value per feature given to the constructor of the tree.
		 */
		void compute_information_gain(double const count_data_points, int* counters[label_count], double* output) const{
			double counts_per_label[label_count] = {0};
			double entropy_leaf = 0;
			for(int l = 0; l < label_count; ++l){
				for(int v = 0; v < tree->features_size[0]; ++v)
					counts_per_label[l] += counters[l][v]; //we use *v* only as index because we just access the first feature
				double const div = (counts_per_label[l] / count_data_points);
				if(div > 0 && !func::isnan(div))
					entropy_leaf += div * func::log2(div);
			}
			entropy_leaf *= -1;

			int start_index = 0;
			for(int f = 0; f < feature_count; ++f){
				//*start_index* is the index where we get the first counter for the feature *f*
				//Remember that output is in the size of *sum_feature_size* minus *feature_count*, so we need to reduce by *f* the output
				compute_entropy(f, start_index, counters, output + start_index - f);
				start_index += tree->features_size[f];
			}
			for(int i = 0; i < (tree->sum_feature_size-feature_count); ++i)
				output[i] = entropy_leaf - output[i];
		}
		/**
		 * Set the limits of the node.
		 * @param new_limits An array that contains the new limits. The size of the array must be at least (tree->sum_feature_size-feature_count).
		 */
		void set_limits(double const* new_limits){
			int* count;
			double* limits;
			double* info_sum;
			get_informations(count, limits, info_sum);
			for(int i = 0; i < (tree->sum_feature_size-feature_count); ++i)
				limits[i] = new_limits[i];
		}

		private:
		/**
		 * Select the split values for each feature based on the upper and lower bound of each feature.
		 * The limits are updated.
		 */
		void select_split_values(void){
			int* count;
			double* limits;
			double* info_sum;
			get_informations(count, limits, info_sum);

			for(int f = 0; f < feature_count; ++f){
				double const width = u_box[f] - l_box[f];
				double const step = width / tree->features_size[f];
				for(int i = 1; i < tree->features_size[f]; ++i){ //start at 1 to avoid having the first limit at *l_box[f]*
					*limits = l_box[f] + i * step;
					limits += 1; //Increase the pointer for the next limit. Remember that the limits are linearly stored
				}
			}
		}
		/**
		 * Overload of new operator. Return a pointer located in *tree.buffer* assuming there is enough space.
		 * Otherwise, it returns nullptr.
		 * @param size The size to allocate (a must have for any new operator overload).
		 * @param tree An additional parameter that indicates the tree of the node.
		 */
		void* operator new(size_t const size, HoeffdingTree& tree){
			int const tmp_offset = tree.buffer_offset + size;
			int const tmp_preset = tree.buffer_preset - compute_total_size_counters(tree.sum_feature_size);
			if(tmp_offset > tmp_preset)
				return nullptr;
			auto ret = tree.buffer+tree.buffer_offset;
			tree.buffer_offset += size;
			return ret;
		}
		/**
		 * Compute the entropy for all the cut on one feature.
		 * The return value is set in the variable *output* and for each cut, *output[i]* contains the entropy knowing the cut i.
		 * @param f The features to study. This value should be less than *feature_count*.
		 * @param start_index The index of the first value of the feature in the counter array. Since feature have variable number of values, we need this index instead of recomputing it.
		 * @param counters The counters for each labels at that node.
		 * @param output An array at least in size of the number of value for the feature *f*. (*features_size[f]*)
		 */
		void compute_entropy(int const f, int const start_index, int* counters[label_count], double* output) const{
			int const f_size = tree->features_size[f];
			int sides[2][label_count] = {0};
			//Compute the sum of the counter in sides[1]
			for(int l = 0; l < label_count; ++l){
				for(int v = 0; v < f_size; ++v){
					sides[1][l] += counters[l][start_index + v];
				}
			}
			//Evaluate each split
			//*sides* will be used to keep up to date the the count for both side of the split.
			for(int v = 0; v < (f_size-1); ++v){
				//*sides* countains the count from both side of the split per label
				//*sum_per_side* contains the same without the label split
				double sum_per_side[2] = {0};
				double probability_per_side[2] = {0};
				for(int l = 0; l < label_count; ++l){
					//The value of the split have move forward so we update sides
					sides[0][l] += counters[l][start_index+v];
					sides[1][l] -= counters[l][start_index+v];
					//Also compute the sum per side
					sum_per_side[0] += sides[0][l];
					sum_per_side[1] += sides[1][l];
				}
				double const sum = sum_per_side[0] + sum_per_side[1];
				probability_per_side[0] = sum_per_side[0] / sum;
				probability_per_side[1] = sum_per_side[1] / sum;

				//We are computing the sum of x*log(x) for each side where x is the probability of the label *l* on both side
				double partial_entropy[2] = {0};
				for(int l = 0; l < label_count; ++l){
					double const div[2] = {(sides[0][l]/sum_per_side[0]), (sides[1][l]/sum_per_side[1])};
					if(div[0] > 0 && !func::isnan(div[0]))
						partial_entropy[0] += div[0] * func::log2(div[0]);
					if(div[1] > 0 && !func::isnan(div[1]))
						partial_entropy[1] += div[1] * func::log2(div[1]);
				}

				//Fix possible NaN values
				if(func::isnan(probability_per_side[0]))
						probability_per_side[0] = 0;
				if(func::isnan(probability_per_side[1]))
						probability_per_side[1] = 0;

				//Finally, we compute the entropy using the *partial_entropy* and the probability of each side
				output[v] = (-1) * partial_entropy[0] * probability_per_side[0] + (-1) * partial_entropy[1] * probability_per_side[1];
				//The result is store in output
			}
		}
		/**
		 * Retrieve the counters  for the node, assuming this is a leaf.
		 * Otherwise, I wouldn't dare imagening what could happen.
		 * @param label The label for which you would like to retrieve the counter.
		 */
		int* get_counters(int& label) const{
			//sum_feature_size * sizeof(int) 		-> bin counters
			int const size_per_label = tree->sum_feature_size * sizeof(int);
			//(sum_feature_size-feature_count) * sizeof(double)	-> limits or cut values considered 
			// 2 time the previous					-> The sum of information gain for each cut
			// sizeof(int) 							-> The number of data point seen so far (easy way :))
			int const fixed_size = sizeof(int) + 2 * (tree->sum_feature_size-feature_count) * sizeof(double);
			int const total_size_counters = fixed_size + size_per_label * label_count;
			//NOTE: split_feature is alway negative when *this* is a leaf.
			//So at the very least, split_feature == -1, therefore, max_size+split_feature is, at most, equal to max_size-1,
			//which is within the size of the array
			int const base_index = max_size + split_feature * total_size_counters;
			int const counters_index = base_index + fixed_size + label * size_per_label;
			
			return reinterpret_cast<int*>(tree->buffer + counters_index);
		}
		/**
		 * Retrieve the counts, the limits and the sum of information gain for each limit for the node, assuming this is a leaf.
		 * Otherwise, I wouldn't dare imagening what could happen.
		 * @param count Output value, the number of data points in that leaf.
		 * @param limits Output value, the limits that delimitate the bins for each feature.
		 * @param informations The sum of information gain for each limit.
		 */
		void get_informations(int*& count, double*& limits, double*& informations) const{
			int const size_per_label = tree->sum_feature_size * sizeof(int);
			int const fixed_size = sizeof(int) + 2 * (tree->sum_feature_size-feature_count) * sizeof(double);
			int const total_size_counters = fixed_size + size_per_label * label_count;

			int const base_index = max_size + split_feature * total_size_counters;
			int const limits_index = base_index + sizeof(int);
			int const info_sum_index = limits_index + (tree->sum_feature_size-feature_count) * sizeof(double);
			count = reinterpret_cast<int*>(tree->buffer + base_index);
			limits = reinterpret_cast<double*>(tree->buffer + limits_index);
			informations = reinterpret_cast<double*>(tree->buffer + info_sum_index);
		}
		/**
		 * Compute the size needed to store the counters, the limits and their information gain values, and the count of data points.
		 */
		int compute_total_size_counters(void) const{
			return Node::compute_total_size_counters(tree->sum_feature_size);
		}
		/**
		 * Compute the size needed to store the counters, the limits and their information gain values, and the count of data points.
		 * @param sum_feature_size The sum of bin for all features.
		 */
		static int compute_total_size_counters(int const sum_feature_size){
			int const size_per_label = sum_feature_size * sizeof(int);
			int const fixed_size = sizeof(int) + 2 * (sum_feature_size-feature_count) * sizeof(double);
			int const total_size_counters = fixed_size + size_per_label * label_count;
			return total_size_counters;
		}
		/**
		 * This function reset all counters and limits to zero.
		 * NOTE: this function is not const because it modifies counter values.
		 * However, cpp would allow this function to be const because these counter values aren't defined as attributes.
		 * Therefore, to keep a bit of coherency, this function should never be set to const.
		 */
		void reset_counters(void){

			int const size_counters = compute_total_size_counters();
			int const starting_index = max_size + split_feature * size_counters;
			for(int i = starting_index; i < starting_index + size_counters; ++i)
				tree->buffer[i] = 0;
		}
		/**
		 * Return the left child of the node assuming the node is not a leaf.
		 */
		Node* get_left_child(void) const{
			return reinterpret_cast<Node*>(tree->buffer) + children[0];
		}
		/**
		 * Return the right child of the node assuming the node is not a leaf.
		 */
		Node* get_right_child(void) const{
			return reinterpret_cast<Node*>(tree->buffer) + children[1];
		}
	};

	/**
	 * The constructor of the Hoeffding Tree.
	 * @param delta The probability of being wrong when choosing a split.
	 * @param features_size The number of bins to use for each feature.
	 */
	HoeffdingTree(double const delta, int const features_size[feature_count]){
		//Compute the sum of values of features
		for(int i = 0; i < feature_count; ++i){
			this->features_size[i] = features_size[i];
			sum_feature_size += features_size[i];
		}
		//This initialize the first node which is set in *buffer*
		Node* root = new (*this) Node(*this);
		this->delta = delta;
	}
	/**
	 * Train all trees of the forest with a new data point.
	 * Return false if a tree has failed to be trained.
	 * @param features The features of the data point of size *feature_count*.
	 * @param label The label of the data point.
	 */
	bool train(feature_type const* features, int const label){
		Node* leaf = sort_in_leaf(features);
		double split_value;
		int const result = leaf->train(features, label, split_value);
		if(result >= 0){
			int const split_result = leaf->split(result, split_value);
			//NOTE: it is very unlikely the node will split, so we don't check
			if(features[result] <= split_value)
				leaf->get_left_child()->train(features, label, split_value);
			else
				leaf->get_right_child()->train(features, label, split_value);
			return (split_result == 0);
		}
		return true;
	}
	/**
	 * Predict the label of the data point.
	 * Return the most likely label.
	 * @param features The features of the data point.
	 * @param scores The score of each label.
	 */
	int predict(feature_type const* features, double* scores = nullptr){
		Node* leaf = sort_in_leaf(features);
		double probabilities[label_count]; //output array of predict doesn't have to be initialized :D.
		leaf->predict(features, probabilities);
		int best = 0;
		for(int i = 1; i < label_count; ++i)
			if(probabilities[i] > probabilities[best])
				best = i;
		//If *scores* is not null, then the function will also return the scores it found for each label.
		if(scores != nullptr)
			for(int l = 0; l < label_count; ++l)
				scores[l] = probabilities[l];
		return best;
	}
	/**
	 * Return the root of the tree.
	 */
	Node* get_root(void){
		return reinterpret_cast<Node*>(buffer);
	}
	private:
	/**
	 * Retrieve the leaf corresponding to the data point.
	 * @param features The data point.
	 */
	Node* sort_in_leaf(feature_type const* features) {
		//Sort into a leaf
		Node* node  = reinterpret_cast<Node*>(buffer);
		int i = 0; //The *i* counter is here to add some security and  make sure we don't loop forever
		while(!node->is_leaf() && i < (max_size/sizeof(Node))){
			int child_index;
			if(features[node->split_feature] <= node->split_value) //below or equal -> left child
				child_index = node->children[0];
			else // right child
				child_index = node->children[1];
			node = reinterpret_cast<Node*>(buffer) + child_index;
			i++;
		}
		return node;
	}
};
