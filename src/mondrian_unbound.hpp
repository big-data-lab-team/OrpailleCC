#include "utils.hpp"
#include <vector>

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
template<class feature_type, class func, int feature_count, int label_count>
class MondrianForestUnbound{
	//Constant to define empty values at some point in the code
	static const int EMPTY_NODE = -1;
	//The node structure
	class MondrianNode {
		protected:
		//The smallest box at that node that contains all training data points who reached that node
		double bound_lower[feature_count];
		double bound_upper[feature_count];
		public:
		//Connexion between nodes.
		//NOTE: if parent == nullptr, the node is a root
		MondrianNode* child_left;
		MondrianNode* child_right;
		MondrianNode* parent;
		double tau;
		/**
		 * Constuctor
		 */
		MondrianNode(){
			for(int i = 0; i < feature_count; ++i)
				bound_lower[i] = bound_upper[i] = 0;
			child_left = child_right = nullptr;
			parent = nullptr;
			tau = EMPTY_NODE;
		}
		/**
		 * Return true if the node is available. Return false if the node is used by one of the trees.
		 */
		bool available(void) const{
			return tau < 0;
		}
		/**
		 * Return true if the node is a leaf.
		 */
		void set_bounds(feature_type const* features){
			//Set the box of the node node_id
			for(int i = 0; i < feature_count; ++i){
				bound_lower[i] = features[i];
				bound_upper[i] = features[i];
			}
		}
		void update_box(feature_type const* features) {
			//update lower bound and upper bound of this node
			for(int i = 0; i < feature_count; ++i){
				if(bound_lower[i] > features[i])
					bound_lower[i] = features[i];
				if(bound_upper[i] < features[i])
					bound_upper[i] = features[i];
			}
		}
		void set_tau(double const v) {
			tau = v;
		}
		double get_lower_bound(int const f) const{
			return bound_lower[f];
		}
		double get_upper_bound(int const f) const{
			return bound_upper[f];
		}
		void set_lower_bound(int const f, double const v){
			bound_lower[f] = v;
		}
		void set_upper_bound(int const f, double const v){
			bound_upper[f] = v;
		}
		double compute_eta(feature_type const* features) const{
			double eta = 0;
			for(int i = 0; i < feature_count; ++i)
				eta += Utils::max(features[i] - bound_upper[i], 0.0) + Utils::max(bound_lower[i] - features[i], 0.0);
			return eta;
		}
	void compute_posterior_mean(double *posterior_mean, double const discount_factor) const{
		//posterior_mean start with the parent posterior_mean and the function updates to the current node
		double const parent_tau = this->parent != nullptr ? this->parent->tau : 0; //The tau value of the parent of the root is 0
		double const node_discount = func::exp(discount_factor * (this->tau - parent_tau));
		double sum_counters = 0;
		double sum_tab = 0;
		double tab[label_count];
		//Compute sum_counters and sum_tab, and set tab
		for(int i = 0; i < label_count; ++i){
			double ci = static_cast<double>(this->get_counters(i));
			sum_counters += ci;
			tab[i] = Utils::min(ci, static_cast<double>(1));
			sum_tab += tab[i];
		}
		//For each label with a counter higher than zero, compute the posterior mean. Otherwise it is simply the posterior mean of the parent
		for(int i = 0; i < label_count; ++i){
			long long int ci = this->get_counters(i);
			if(ci > 0){
				double const c = static_cast<double>(ci);
				double const a = c - node_discount * tab[i];
				double const b = node_discount * sum_tab * posterior_mean[i];
				posterior_mean[i] = (a / sum_counters) + (b /sum_counters);
			}
		}
	}
		virtual void update_counters(int const label, long long int const qty)=0;
		virtual void update_counters(void)=0;
		virtual long long int get_counters(int const label) const=0;
		virtual bool is_leaf(void) const =0;
		virtual ~MondrianNode(){
			delete child_left;
			delete child_right;
		}
	};
	class LeafNode : public MondrianNode{
		//The counters of each labels that reach that node.
		long long int counters[label_count];
		public:
		virtual bool is_leaf(void) const{
			return true;
		}
		/**
		 * Constuctor
		 */
		LeafNode(){
			for(int i = 0; i < label_count; ++i)
				counters[i] = 0;
		}
		virtual void update_counters(int const label, long long int const qty){
			counters[label] += qty;
			if (this->parent != nullptr)
				this->parent->update_counters(label, qty);
		}
		virtual void update_counters(void){
			if (this->parent != nullptr)
				this->parent->update_counters();
		}
		virtual long long int get_counters(int const label) const{
			return counters[label];
		}
	};
	class InternalNode : public MondrianNode{
		//The features used for the split
		int split_dimension;
		//The value used for the split
		double split_value;
		//The counters of each labels that reach that node. At most equal to 2
		char counters[label_count];
		public:
		InternalNode(){
			for(int i = 0; i < label_count; ++i)
				counters[i] = 0;
			split_dimension = EMPTY_NODE;
			split_value = EMPTY_NODE;
		}
		void set_split_dimension(int const sd){
			split_dimension = sd;
		}
		void set_split_value(double const sv){
			split_value = sv;
		}
		int get_split_dimension(void) const{
			return split_dimension;
		}
		double get_split_value(void) const{
			return split_value;
		}
		bool point_go_left(feature_type const* features) const{
			return features[split_dimension] <= split_value;
		}
		virtual bool is_leaf(void) const{
			return false;
		}
		virtual void update_counters(int const label, long long int const qty){
			//if(counters[label] == 2)
				//return;
			long long int l = Utils::min(static_cast<long long int>(1), this->child_left->get_counters(label));
			long long int r = Utils::min(static_cast<long long int>(1), this->child_right->get_counters(label));
			char cleft = (char) l;
			char cright = (char) r;
			counters[label] = cleft+cright;
			if(this->parent != nullptr)
				this->parent->update_counters(label, qty);
		}
		virtual void update_counters(void){
			for(int label = 0; label < label_count; ++label){
				long long int l = Utils::min(static_cast<long long int>(1), this->child_left->get_counters(label));
				long long int r = Utils::min(static_cast<long long int>(1), this->child_right->get_counters(label));
				char cleft = (char) l;
				char cright = (char) r;
				counters[label] = cleft+cright;
			}
			if (this->parent != nullptr)
				this->parent->update_counters();
		}
		virtual long long int get_counters(int const label) const{
			return (long long int) counters[label];
		}
	};
	//The node_id of the roots of each tree
	vector<MondrianNode*> roots;
	//The lifetime parameter
	double lifetime;
	//The base measure parameter
	double base_measure;
	//The discount factor parameter
	double discount_factor;
	int tree_count;
	long long int count_internal_nodes = 0;
	long long int count_leaf_nodes = 0;
	long long int count_data_point = 0;

	/**
	 * Given a node, apply the extend algorithm described in the Mondrian paper.
	 * @param node_id The index of the node in the array *nodes*.
	 * @param tree_id The id of the tree the node belongs. This is used to check if the node is the root or not.
	 * @param features The features of the new data point.
	 * @param label The label of the new data  point.
	 */
	void extend_block(MondrianNode* node, int const tree_id, feature_type const* features, int const label){
		//e_lower and e_upper are used to compute probabilities
		feature_type e_lower[feature_count], e_upper[feature_count];
		double probabilities[feature_count];
		MondrianNode* parent = node->parent;
		double const parent_tau = parent != nullptr ? parent->tau : 0; //The tau value of the parent of the root is 0
		//sum is used as a parameter to pick random numbers following exponential law
		feature_type sum = 0;
		//compute e_lower, e_upper and sum
		for(int i = 0; i < feature_count; ++i){
			e_lower[i] = node->get_lower_bound(i) - features[i] > 0 ? node->get_lower_bound(i) - features[i] : 0;
			e_upper[i] = features[i] - node->get_upper_bound(i) > 0 ? features[i] - node->get_upper_bound(i) : 0;
			probabilities[i] = e_lower[i] + e_upper[i];
			sum += e_lower[i] + e_upper[i];
		}
		//Pick a random number following an exponential law of parameter *sum* (except if sum is 0)
		double const E = sum == 0 ?  -1 : Utils::rand_exponential<func>(sum);
		bool update_box = false;
		if(E >= 0 && parent_tau + E < node->tau){//Introduce a new parent and a new sibling
			Utils::turn_array_into_probability(probabilities, feature_count, sum);
			//sample features with probability proportional to e_lower[i] + e_upper[i]
			int const dimension = Utils::pick_from_distribution<func>(probabilities, feature_count);

			//Select the bound to choose the split from
			double lower_value, upper_value;
			if(features[dimension] > node->get_upper_bound(dimension)){
				lower_value = node->get_upper_bound(dimension);
				upper_value = features[dimension];
			}
			else if(features[dimension] < node->get_lower_bound(dimension)){
				lower_value = features[dimension];
				upper_value = node->get_lower_bound(dimension);
			}

			//sample the split between [lower_value, upper_value]
			double const split_value = func::rand_uniform()*(upper_value - lower_value) + lower_value;
			InternalNode* new_parent = new InternalNode();
			LeafNode* new_sibling = new LeafNode();
			count_internal_nodes += 1;
			count_leaf_nodes += 1;
			//insert new node above the current one
			new_parent->set_split_dimension(dimension);
			new_parent->set_split_value(split_value);
			new_parent->tau = parent_tau + E;

			//insert new leaf, sibbling of the current one

			//Update the box of the new parent
			for(int i = 0; i < feature_count; ++i){
				new_parent->set_lower_bound(i, features[i] < node->get_lower_bound(i) ? features[i] : node->get_lower_bound(i));
				new_parent->set_upper_bound(i, features[i] > node->get_upper_bound(i) ? features[i] : node->get_upper_bound(i));
			}
			//NOTE Creates counters for the label of the new parent
			//No need to increase the counter for the current label because we will call sample_block soon on new_sibling

			//Make the connections between the new nodes
			new_parent->parent = node->parent;
			if(node->parent == nullptr && node == roots[tree_id])//We introduce a parent to the root
				roots[tree_id] = new_parent;
			else{
				MondrianNode* parent = node->parent;
				if(parent->child_left == node)
					parent->child_left = new_parent;
				else
					parent->child_right = new_parent;
			}


			node->parent = new_parent;

			new_sibling->parent = new_parent;
			if(features[dimension] == upper_value){ //right
				new_parent->child_right = new_sibling;
				new_parent->child_left = node;
			}
			else{ //left
				new_parent->child_left = new_sibling;
				new_parent->child_right = node;
			}

			//cout << "Split dimension " << dimension << " on " << split_value << "(data point: " << count_data_point << endl;
			sample_block(new_sibling, features, label);
			new_parent->update_counters();

		}
		else{
			update_box = true;
		}

		if(update_box){ //Otherwise, just update the box
			node->update_box(features);
			//if not leaf, recurse on the node that contains the data point
			if(!node->is_leaf()){
				InternalNode* inode = dynamic_cast<InternalNode *>(node);
				if(inode->point_go_left(features))
					extend_block(inode->child_left, tree_id, features, label);
				else
					extend_block(inode->child_right, tree_id, features, label);
				//NOTE: we don't update the counters of labels here because the counting will be done when prediction is required.
				//We can optimize that.
			}
			else{
				LeafNode* lnode = dynamic_cast<LeafNode *>(node);
				//Update the counter of label
				lnode->update_counters(label, 1);

			}
		}
	}
	/**
	 * Given a node, apply the sample algorithm described in the Mondrian paper..
	 * @param node_id The index of the node in the array *nodes*.
	 * @param features The features of the new data point.
	 * @param label The label of the new data  point.
	 */
	void sample_block(LeafNode* node, feature_type const* features, int const label){
		node->set_bounds(features);
		//Update the counter for labels
		node->update_counters(label, 1);
		//NOTE we don't need to check for lifetime because we only have one element
		//see the original source code (https://github.com/balajiln/mondrianforest)
		node->set_tau(lifetime);
	}
	/**
	 * Train tree *tree_id* with a new data point. If the tree does not exist, it will be created.
	 * Returns true if the training went well.
	 * @param features The features of the new data point.
	 * @param label The label of the new data  point.
	 * @param tree_id The id of the tree which is an index between 0 and tree_count.
	 */
	bool train_tree(feature_type const* features, int const label, int const tree_id){
		MondrianNode* root = roots[tree_id];
		if (root == nullptr){ //The root of the tree does not exist yet
			//Initialize this node as the root for this tree
			LeafNode* leafroot = new LeafNode();
			roots[tree_id] = leafroot;
			count_leaf_nodes += 1;

			//Sample the root with the new data point
			sample_block(leafroot, features, label);
		}
		else{ //Partial fit
			extend_block(root, tree_id, features, label);
		}
		return true;
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
		MondrianNode* current_node = roots[tree_id];
		for(int i = 0; i < label_count; ++i)
			posterior_means[i] = base_measure;

		double pp_sum = 0, pp_mul = 1;
		double probability_of_branching;
		double probability_not_separated_yet = 1;
		double parent_tau = 0;

		double smoothed_posterior_means[label_count] = {0};

		//Find the corresponding leaf for the data point
		while(current_node != nullptr) {
			double const delta_tau = current_node->tau - parent_tau;
			double const eta = current_node->compute_eta(features);
			probability_of_branching = 1 - func::exp(-delta_tau * eta);

			if(probability_of_branching > 0){
				double const new_node_discount = (eta / (eta + discount_factor)) *
												-Utils::expm1<func>(-(eta + discount_factor) * delta_tau) / -Utils::expm1<func>(-(eta * delta_tau));
				double c[label_count];
				double c_sum = 0;
				//We need the sum of *c*, so we need two loops
				for(int l = 0; l < label_count; ++l){
					c[l] = Utils::min(static_cast<double>(current_node->get_counters(l)), static_cast<double>(1));
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
			current_node->compute_posterior_mean(posterior_means, discount_factor);

			//If we reach a leaf, and *posterior_means* will be set to the value for this leaf.
			if(current_node->is_leaf()){
				for(int l = 0; l < label_count; ++l)
					posterior_means[l] = smoothed_posterior_means[l] + probability_not_separated_yet * (1 - probability_of_branching) * posterior_means[l];
				break;
			}
			InternalNode* inode = dynamic_cast<InternalNode*>(current_node);
			probability_not_separated_yet *= (1 - probability_of_branching);

			//Otherwise, the child is picked based on the split of the node
			if(inode->point_go_left(features))
				current_node = inode->child_left;
			else
				current_node = inode->child_right;
		}
	}
	public:
	/**
	 * Constructor.
	 * @param lifetime The lambda parameter described in the Mondrian paper. It is used to limit the growth of the tree.
	 * @param base_measure A parameter that set the initial posterior means of the parent of the root.
	 * @param discount_factor The discount factor :).
	 */
	MondrianForestUnbound(double const lifetime, double const base_measure, double const discount_factor, int const tree_count){
		this->lifetime = lifetime;
		this->base_measure = base_measure;
		this->discount_factor = discount_factor;
		this->tree_count = tree_count;
		//Init all roots as empty
		roots.resize(tree_count, nullptr);
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
		count_data_point += 1;
		return fully_trained;
	}
	/**
	 * Predict the label of the data point.
	 * Return the most likely label.
	 * @param features The features of the data point.
	 */
	int predict(feature_type const* features, double* scores = nullptr){
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
	~MondrianForestUnbound(){
		for(MondrianNode* root : roots)
			if(root != nullptr)
				delete root;
	}
};

