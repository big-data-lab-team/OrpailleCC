#include "utils.hpp"

/**
 * MondrianForest class implements the Mondrian Forest classifier.
 * Templates:
 * - feature_type: The type of the features of the data points
 * - func: a class type that contains all needed function for the Mondrian Forest.
 *   	+ exp function: A function that compute the exponential of a double. (Used to compute the posterior means)
 *   	+ rand_uniform function: A function that pick uniformly a random number between [0,1[.
 *   	+ log function: A function that run the natural logarithm.
 * - tree_count: the number of tree to use in the forest.
 * - feature_count: the number of features for one data point.
 * - label_count: the number of different labels.
 * - max_size: the maximum size of the forest in bytes
 */
template<class feature_type, class func, int tree_count, int feature_count, int label_count, int max_size>
class MondrianForest{
	//Constant to define empty values at some point in the code
	static const int EMPTY_NODE = -1;
	//The node structure
	struct Node{
		//The features used for the split
		int split_dimension;
		//The value used for the split
		double split_value;
		//The smallest box at that node that contains all training data points who reached that node
		double bound_lower[feature_count];
		double bound_upper[feature_count];

		//Connexion between nodes.
		//NOTE: if parent == EMPTY_NODE, the node is a root
		int child_left, child_right, parent;
		//The lifetime of the node. This parameter control the growth of a tree.
		double tau;
		//The counters of each labels that reach that node.
		int counters[label_count];
		/**
		 * Constuctor
		 */
		Node(){
			for(int i = 0; i < label_count; ++i)
				counters[i] = 0;
			child_left = child_right = EMPTY_NODE;
			split_dimension = EMPTY_NODE;
			tau = EMPTY_NODE;
			parent = EMPTY_NODE;
		}
		/**
		 * Copy Constuctor
		 */
		Node(Node const& src){
			split_dimension = src.split_dimension;
			split_value = src.split_value;
			child_left = src.child_left;
			child_right = src.child_right;
			tau = src.tau;
			for(int i = 0; i < label_count; ++i)
				counters[i] = src.counters[i];
			for(int i = 0; i < feature_count; ++i){
				bound_lower[i] = src.bound_lower[i];
				bound_upper[i] = src.bound_upper[i];
			}
		}
		#ifdef DEBUG
		void print(bool all = false) const{
			cout << "Split dim: " << split_dimension << "\tSplit val: " << split_value << endl;
			cout << "Parent: " << parent << "\tChild: " << child_left << ", " << child_right << endl;
			cout << "Tau: " << tau << endl;
			cout << "[" << counters[0];
			for(int i = 1; i < label_count; ++i)
				cout << ", " << counters[i];
			cout << "]" << endl;
			if(all){
				cout << "Box =>" << endl;
				for(int i = 0; i < feature_count; ++i)
					cout << "[" << bound_lower[i] << ", " << bound_upper[i] << "]" << endl;
			}
		}
		#endif
		/**
		 * Return true if the node is available. Return false if the node is used by one of the trees.
		 */
		bool available(void) const{
			return tau < 0;
		}
		/**
		 * Return true if the node is a leaf.
		 */
		bool is_leaf(void) const{
			return split_dimension == EMPTY_NODE;
		}
		double compute_branching_probability(feature_type const* features) const{
			return -1;
		}
	};
	//The maximum number of nodes
	static const int MAX_NODE = (max_size - (max_size%sizeof(Node))) / sizeof(Node);
	//The pool of node to use.
	Node nodes[MAX_NODE];
	//The node_id of the roots of each tree
	int roots[tree_count];
	//The lifetime parameter
	double lifetime;
	//The base measure parameter
	double base_measure;
	//The discount factor parameter
	double discount_factor;
	/**
	 *	Return the index of an empty node.
	 */
	int available_node(void) const{
		for(int i = 0; i < MAX_NODE; ++i)
			if(nodes[i].available())
				return i;
		return -1;
	}
	/**
	 * Given a node, apply the extend algorithm described in the Mondrian paper.
	 * @param node_id The index of the node in the array *nodes*.
	 * @param tree_id The id of the tree the node belongs. This is used to check if the node is the root or not.
	 * @param features The features of the new data point.
	 * @param label The label of the new data  point.
	 */
	void extend_block(int const node_id, int const tree_id, feature_type const* features, int const label){
		//e_lower and e_upper are used to compute probabilities
		feature_type e_lower[feature_count], e_upper[feature_count];
		double probabilities[feature_count];
		Node& node = nodes[node_id];
		int const parent_id = node.parent;
		double const parent_tau = node.parent >= 0 ? nodes[node.parent].tau : 0; //The tau value of the parent of the root is 0
		//sum is used as a parameter to pick random numbers following exponential law
		feature_type sum = 0;
		//compute e_lower, e_upper and sum
		for(int i = 0; i < feature_count; ++i){
			e_lower[i] = node.bound_lower[i] - features[i] > 0 ? node.bound_lower[i] - features[i] : 0;
			e_upper[i] = features[i] - node.bound_upper[i] > 0 ? features[i] - node.bound_upper[i] : 0;
			probabilities[i] = e_lower[i] + e_upper[i];
			sum += e_lower[i] + e_upper[i];
		}
		//Pick a random number following an exponential law of parameter *sum* (except if sum is 0)
		double const E = sum == 0 ?  -1 : Utils::rand_exponential<func>(sum);
		bool update_box = false;
		if(E >= 0 && parent_tau + E < node.tau){//Introduce a new parent and a new sibling
			Utils::turn_array_into_probability(probabilities, feature_count, sum);
			//sample features with probability proportional to e_lower[i] + e_upper[i]
			int dimension = Utils::pick_from_distribution<func>(probabilities, feature_count);
			if(dimension >= feature_count || dimension < 0){
				dimension = static_cast<int>(func::rand_uniform() * static_cast<double>(feature_count));
			}

			//Select the bound to choose the split from
			double lower_value, upper_value;
			if(features[dimension] > node.bound_upper[dimension]){
				lower_value = node.bound_upper[dimension];
				upper_value = features[dimension];
			}
			else if(features[dimension] < node.bound_lower[dimension]){
				lower_value = features[dimension];
				upper_value = node.bound_lower[dimension];
			}

			//sample the split between [lower_value, upper_value]
			double const split_value = func::rand_uniform()*(upper_value - lower_value) + lower_value;
			int new_parent, new_sibling;
			//insert new node above the current one
			new_parent = available_node();
			if(new_parent >= 0){
				nodes[new_parent].split_dimension = dimension;
				nodes[new_parent].split_value = split_value;
				nodes[new_parent].tau = parent_tau + E;

				//insert new leaf, sibbling of the current one
				new_sibling = available_node();
				if(new_sibling >= 0){

					//Update the box of the new parent
					for(int i = 0; i < feature_count; ++i){
						nodes[new_parent].bound_lower[i] = features[i] < node.bound_lower[i] ? features[i] : node.bound_lower[i];
						nodes[new_parent].bound_upper[i] = features[i] > node.bound_upper[i] ? features[i] : node.bound_upper[i];
					}
					//NOTE Creates counters for the label of the new parent
					//for(int i = 0; i < label_count; ++i)
					//nodes[new_parent].counters[i] = node.counters[i];
					//nodes[new_parent].counters[label] += 1;

					//Creates counters for the label of the new sibling
					for(int i = 0; i < label_count; ++i)
						nodes[new_sibling].counters[i] = 0;
					//No need to increase the counter for the current label because we will call sample_block soon on new_sibling

					//Make the connections between the new nodes
					nodes[new_parent].parent = node.parent;
					if(node.parent == EMPTY_NODE && node_id == roots[tree_id])//We introduce a parent to the root
						roots[tree_id] = new_parent;
					else{
						Node& parent = nodes[node.parent];
						if(parent.child_left == node_id)
							parent.child_left = new_parent;
						else
							parent.child_right = new_parent;
					}


					node.parent = new_parent;

					nodes[new_sibling].parent = new_parent;
					if(features[dimension] == upper_value){ //right
						nodes[new_parent].child_right = new_sibling;
						nodes[new_parent].child_left = node_id;
					}
					else{ //left
						nodes[new_parent].child_left = new_sibling;
						nodes[new_parent].child_right = node_id;
					}

					sample_block(new_sibling, features, label);
				}
				else{
					update_box = true;
				}
			}
			else{
				update_box = true;
			}
		}
		else{
			update_box = true;
		}

		if(update_box){ //Otherwise, just update the box
			//update lower bound and upper bound of this node
			for(int i = 0; i < feature_count; ++i){
				if(node.bound_lower[i] > features[i])
					node.bound_lower[i] = features[i];
				if(node.bound_upper[i] < features[i])
					node.bound_upper[i] = features[i];
			}
			//if not leaf, recurse on the node that contains the data point
			if(!node.is_leaf()){
				if(features[node.split_dimension] <= node.split_value)
					extend_block(node.child_left, tree_id, features, label);
				else if(features[node.split_dimension] > node.split_value)
					extend_block(node.child_right, tree_id, features, label);
				//NOTE: we don't update the counters of labels here because the counting will be done when prediction is required.
				//We can optimize that.
			}
			else{
				//Update the counter of label
				node.counters[label] += 1;
			}
		}
	}
	/**
	 * Given a node, apply the sample algorithm described in the Mondrian paper..
	 * @param node_id The index of the node in the array *nodes*.
	 * @param features The features of the new data point.
	 * @param label The label of the new data  point.
	 */
	void sample_block(int const node_id, feature_type const* features, int const label){
		Node& node = nodes[node_id];
		//Set the box of the node node_id
		for(int i = 0; i < feature_count; ++i){
			node.bound_lower[i] = features[i];
			node.bound_upper[i] = features[i];
		}
		//Update the counter for labels
		node.counters[label] += 1;
		//NOTE we don't need to check for lifetime because we only have one element
		//see the original source code (https://github.com/balajiln/mondrianforest)
		node.tau = lifetime;
	}
	/**
	 * Train tree *tree_id* with a new data point. If the tree does not exist, it will be created.
	 * Returns true if the training went well.
	 * @param features The features of the new data point.
	 * @param label The label of the new data  point.
	 * @param tree_id The id of the tree which is an index between 0 and tree_count.
	 */
	bool train_tree(feature_type const* features, int const label, int const tree_id){
		int root_id = roots[tree_id];
		if (root_id == EMPTY_NODE){ //The root of the tree does not exist yet
			//Pick a new node
			root_id = available_node();

			//Initialize this node as the root for this tree
			roots[tree_id] = root_id;
			Node& root = nodes[root_id];
			root.parent = -1;
			root.child_right = -1;
			root.child_left = -1;
			root.tau = 0;

			//Sample the root with the new data point
			sample_block(root_id, features, label);
		}
		else{ //Partial fit
			extend_block(root_id, tree_id, features, label);
		}
		return true;
	}
	/**
	 * Compute the posterior mean at a node based on the posterior mean of its parent.
	 * @param node A reference of the node.
	 * @param posterior_mean The posterior mean of the parent. It will be turned into the posterior mean of the node.
	 */
	void compute_posterior_mean(Node const& node, double *posterior_mean) const{
		//posterior_mean start with the parent posterior_mean and the function updates to the current node
		double const parent_tau = node.parent >= 0 ? nodes[node.parent].tau : 0; //The tau value of the parent of the root is 0
		double const node_discount = func::exp(discount_factor * (node.tau - parent_tau));
		double sum_counters = 0;
		double sum_tab = 0;
		double tab[label_count];
		//Compute sum_counters and sum_tab, and set tab
		for(int i = 0; i < label_count; ++i){
			sum_counters += node.counters[i];
			tab[i] = Utils::min(node.counters[i], 1);
			sum_tab += tab[i];
		}
		//For each label with a counter higher than zero, compute the posterior mean. Otherwise it is simply the posterior mean of the parent
		for(int i = 0; i < label_count; ++i){
			if(node.counters[i] > 0){
				double const a = node.counters[i] - node_discount * tab[i];
				double const b = node_discount * sum_tab * posterior_mean[i];
				posterior_mean[i] = (a / sum_counters) + (b /sum_counters);
			}
		}
	}
	/**
	 * Return the posterior means of the leaf of a data point in the tree *tree_id*. This is the prediction of the tree.
	 * The data point go through the tree until it reach a leaf, then the posterior means of this leaf (or its virtual sibling) is returned.
	 * The virtual sibling can appears if the new data point fell out of the box of the leaf (not implemented yet).
	 * @param features The data point.
	 * @param tree_id The id of the tree which is an index between 0 and tree_count.
	 * @param posterior_mean An array in the size of label_count, that will hold the posterior means.
	 */
	void predict_tree(feature_type const* features, int const tree_id, double* posterior_means) const{
		int node_id = roots[tree_id];
		for(int i = 0; i < label_count; ++i)
			posterior_means[i] = base_measure;

		double pp_sum = 0, pp_mul = 1;
		double probability_of_branching;
		double probability_not_separated_yet = 1;
		double parent_tau = 0;

		double smoothed_posterior_means[label_count] = {0};

		//Find the corresponding leaf for the data point
		while(node_id != EMPTY_NODE) {
			Node const& current_node = nodes[node_id];

			double const delta_tau = current_node.tau - parent_tau;
			double eta = 0;
			for(int i = 0; i < feature_count; ++i)
				eta += Utils::max(features[i] - current_node.bound_upper[i], 0.0) + Utils::max(current_node.bound_lower[i] - features[i], 0.0);

			probability_of_branching = 1 - func::exp(-delta_tau * eta);

			if(probability_of_branching > 0){
				double const new_node_discount = (eta / (eta + discount_factor)) *
												-Utils::expm1<func>(-(eta + discount_factor) * delta_tau) / -Utils::expm1<func>(-(eta * delta_tau));
				double c[label_count];
				double c_sum = 0;
				//We need the sum of *c*, so we need two loops
				for(int l = 0; l < label_count; ++l){
					c[l] = Utils::min(current_node.counters[l], 1);
					c_sum += c[l];
				}

				for(int l = 0; l < label_count; ++l){
					//posterior_means of the parent of current_node
					double const posterior_mean = (1/c_sum) * (c[l] - new_node_discount * c[l] + c_sum * posterior_means[l]);
					//Note that *posterior_mean* is the value for the hypothetical parent
					smoothed_posterior_means[l] += probability_not_separated_yet * probability_of_branching * posterior_mean;
				}
			}

			//NOTE: *posterior_means* cannot be update before we need the parent value above
			compute_posterior_mean(current_node, posterior_means);

			//If we reach a leaf, and *posterior_means* will be set to the value for this leaf.
			if(current_node.is_leaf()){
				for(int l = 0; l < label_count; ++l)
					posterior_means[l] = smoothed_posterior_means[l] + probability_not_separated_yet * (1 - probability_of_branching) * posterior_means[l];
				break;
			}
			probability_not_separated_yet *= (1 - probability_of_branching);

			//Otherwise, the child is picked based on the split of the node
			if(features[current_node.split_dimension] <= current_node.split_value)
				node_id = current_node.child_left;
			else
				node_id = current_node.child_right;
		}
	}
	/**
	 * Update the label counters of an internal node.
	 * It is a recursive function so if apply to the root, it will apply to an entire tree.
	 * @param node_id The id of the node to update.
	 */
	void update_posterior_count(int const node_id){
		Node& node = nodes[node_id];
		if(node.is_leaf()){
			//A leaf should already has its count right
			return;
		}
		else{ //This is an internal node
			//Compute the posterior count for the two children
			update_posterior_count(node.child_left);
			update_posterior_count(node.child_right);
		}
		//Update each count
		for(int i = 0; i < label_count; ++i){
			int const c_left = Utils::min(1, nodes[node.child_left].counters[i]);
			int const c_right = Utils::min(1, nodes[node.child_right].counters[i]);
			node.counters[i] = c_left + c_right;
		}
	}
	/**
	 * Update the label counters of all trees.
	 */
	void update_posterior_count(void){
		//For each tree, run the recursive *update_posterior_count* on the root
		for(int i = 0; i < tree_count; ++i)
			if(roots[i] != EMPTY_NODE)
				update_posterior_count(roots[i]);
	}
	public:
	/**
	 * Constructor.
	 * @param lifetime The lambda parameter described in the Mondrian paper. It is used to limit the growth of the tree.
	 * @param base_measure A parameter that set the initial posterior means of the parent of the root.
	 * @param discount_factor The discount factor :).
	 */
	MondrianForest(double const lifetime, double const base_measure, double const discount_factor){
		this->lifetime = lifetime;
		this->base_measure = base_measure;
		this->discount_factor = discount_factor;
		//Init all roots as empty
		for(int i = 0; i < tree_count; ++i)
			roots[i] = EMPTY_NODE;
	}
	/**
	 * Train all trees of the forest with a new data point.
	 * Return false if a tree has failed to be trained.
	 * @param features The features of the data point of size *feature_count*.
	 * @param label The label of the data point.
	 */
	bool train(feature_type const* features, int const label){
		bool fully_trained;
		for(int i = 0; i < tree_count; ++i){
			bool has_trained = train_tree(features, label, i);
			if(!has_trained)
				fully_trained = false;
		}
		return fully_trained;
	}
	/**
	 * Predict the label of the data point.
	 * Return the most likely label.
	 * @param features The features of the data point.
	 */
	int predict(feature_type const* features, double* scores = nullptr){
		//Update internal count
		update_posterior_count();

		//The posterior mean of the forest will be the average posterior means over all trees
		//We start by computing the sum
		double sum_posterior_mean[label_count] = {0};
		for(int i = 0; i < tree_count; ++i){
			double posterior_mean[label_count];
			//Get the  posterior means of the leaf of the data point in tree *i*
			predict_tree(features, i, posterior_mean);
			//Update the sum of posterior means
			for(int k = 0; k < label_count; ++k)
				sum_posterior_mean[k] += posterior_mean[k];
		}
		//Then we divide by the number trees.
		for(int k = 0; k < label_count; ++k)
			sum_posterior_mean[k] /= static_cast<double>(tree_count);

		//Finally, we look for the best label
		int best = 0;
		for(int i = 1; i < label_count; ++i){
			if(sum_posterior_mean[i] > sum_posterior_mean[best])
				best = i;
		}
		return best;
	}
};
