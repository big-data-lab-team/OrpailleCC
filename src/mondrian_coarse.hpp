#include "utils.hpp"
#include "metrics.hpp"
#include <typeinfo>
#include <iostream>
using namespace std;
#ifdef DEBUG
#include <iostream>
using namespace std;
#endif
template<int feature_count, int label_count>
struct MondrianNode{
	//Constant to define empty values at some point in the code
	static const int EMPTY_NODE = -1;
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
	//Counter of element that should have been out of the box, but couldn't because of lack of memory
	int forced_extend;
	//Contain a score for the node.
	double fading_score;

	/**
	 * Constuctor
	 */
	MondrianNode(){
		reset();
	}
	void chop(void){
		child_left = child_right = EMPTY_NODE;
		split_dimension = EMPTY_NODE;
		split_value = 0;
	}
	void reset(void){
		for(int i = 0; i < label_count; ++i)
			counters[i] = 0;
		for(int i = 0; i < feature_count; ++i)
			bound_lower[i] = bound_upper[i] = 0;
		child_left = child_right = EMPTY_NODE;
		split_dimension = EMPTY_NODE;
		split_value = 0;
		tau = EMPTY_NODE;
		parent = EMPTY_NODE;
		forced_extend = 0;
		fading_score = 0;
	}
	bool has_parent(void) const{
		return parent != EMPTY_NODE;
	}
	/**
	 * Copy Constuctor
	 */
	MondrianNode(MondrianNode const& src){
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
	void operator=(MondrianNode const& node){
		if(&node != this){
			split_dimension = node.split_dimension;
			split_value = node.split_value;
			child_left = node.child_left;
			child_right = node.child_right;
			parent = node.parent;
			tau = node.tau;
			for(int i = 0; i < feature_count; ++i){
				bound_lower[i] = node.bound_lower[i];
				bound_upper[i] = node.bound_upper[i];
			}
			for(int i = 0; i < label_count; ++i){
				counters[i] = node.counters[i];
			}
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
	double compute_branching_probability(double const* features) const{
		return -1;
	}
};
#define DO_DELETE 0
#define DONT_DELETE 1

#define DEPTH_SIZE 0
#define NODE_SIZE 1

#define COBBLE_MANAGEMENT 0
#define OPTIMISTIC_COBBLE_MANAGEMENT 1
#define ROBUR_MANAGEMENT 2
#define PHOENIX_MANAGEMENT 3
#define PAUSING_PHOENIX_MANAGEMENT 4

#define FE_DISTRIBUTION_ZERO 0
#define FE_DISTRIBUTION_SPLIT 1
#define FE_DISTRIBUTION_PROPORTIONAL 2
#define FE_DISTRIBUTION_DECREMENT 3

#define SPLIT_TRIGGER_NONE 0
#define SPLIT_TRIGGER_POSITIVE 1
#define SPLIT_TRIGGER_TOTAL 2
#define SPLIT_TRIGGER_SFE 3

#define SPLIT_HELPER_NONE 0
#define SPLIT_HELPER_AVG 1
#define SPLIT_HELPER_WEIGHTED 2

#define EXTEND_NONE 0
#define EXTEND_ORIGINAL 1
#define EXTEND_GHOST 2
#define EXTEND_PARTIAL_UPDATE 3
#define EXTEND_COUNTER_NO_UPDATE 4
#define EXTEND_BARYCENTER 5

#define TRIM_NONE 0
#define TRIM_RANDOM 1
#define TRIM_FADING 2
#define TRIM_COUNT 3


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
template<class feature_type, class func, class Statistic, int feature_count, int label_count, int max_size>
class CoarseMondrianForest{
typedef MondrianNode<feature_count, label_count> Node;


int const tree_management;
int const size_type;
int size_limit;
int const dont_delete;
bool const print_nodes;
int const fe_distribution;
int const fe_split_trigger;
double const tau_factor;
bool const generate_full_point;
bool const reset_once;
double const fe_parameter;
int const split_helper;
int const extend_type;
int const trim_type;

//The node structure
	public:
struct TreeBase{
	static const int EMPTY_ROOT = -1;
	int node_count_limit = 0;
	int size = 0;
	int root;
	double sum_contribution = 0;
	double count_contribution = 0;
	Statistic statistics;
	bool is_empty(void) const{
		return root == EMPTY_ROOT;
	}
	void reset(int const node_limit=1){
		root = EMPTY_ROOT;
		node_count_limit = node_limit;
		statistics.reset();
		sum_contribution = 0;
		count_contribution = 0;
	}
	bool is_paused(int const tree_management = 0) const{
		if(tree_management == ROBUR_MANAGEMENT || tree_management == PAUSING_PHOENIX_MANAGEMENT)
			return size >= node_count_limit;
		else if(tree_management == COBBLE_MANAGEMENT || tree_management == OPTIMISTIC_COBBLE_MANAGEMENT)
			return false; //
		else if(tree_management == PHOENIX_MANAGEMENT)
			return false;
		return false;
	}
	bool is_grown(int const tree_management = 0) const{
		if(tree_management == COBBLE_MANAGEMENT || tree_management == OPTIMISTIC_COBBLE_MANAGEMENT)
			return size >= node_count_limit; //
		return is_paused(tree_management);
	}
};
private:

unsigned char buffer[max_size];

unsigned int total_count = 0;

double count_direction = 0;
double sum_direction = 0;
int count_full = 0;

int last_tree_deleted = -1;
int node_usage_on_ltd = 0;
//The number of nodes
int node_count = 0;
int node_available = 0;
//The number of trees
int tree_count = 0;
int maximum_tree_count = 0;
//The lifetime parameter
double lifetime;
//The base measure parameter
double base_measure;
//The discount factor parameter
double discount_factor;
double sum_features[feature_count]; //Sum of all data points
double count_points = 0; // count of points for the average point
double fading_count = 1;//fading factor for the average point
double nodes_fading_f = 0.995; //Fading factor of node
double maximum_trim_size = 1.00;
int has_been_full = 0;


Node* nodes() {
	return reinterpret_cast<Node*>(buffer);
}
Node const* nodes() const {
	return reinterpret_cast<Node const*>(buffer);
}
TreeBase* tree_bases() {
	return reinterpret_cast<TreeBase*>(buffer + max_size - tree_count * sizeof(TreeBase));
}
TreeBase const* tree_bases() const{
	return reinterpret_cast<TreeBase const*>(buffer + max_size - tree_count * sizeof(TreeBase));
}
/**
 *	Return the index of an empty node.
 */
int available_node(void) const{
	if(node_available == 0)
		return -1;
	static int last_returned_node = 0;
	for(int i = 0; i < node_count; ++i){
		int const node_id = (i + last_returned_node) % node_count;
		if(nodes()[node_id].available()){
			last_returned_node = node_id;
			return node_id;
		}
	}
	return -1;
}
/**
 * Given a node, apply the extend algorithm described in the Mondrian paper.
 * @param node_id The index of the node in the array *nodes*.
 * @param tree_id The id of the tree the node belongs. This is used to check if the node is the root or not.
 * @param features The features of the new data point.
 * @param label The label of the new data  point.
 */
//Original extend
void extend_block0(int const node_id, int const tree_id, feature_type const* features, int const label){
	//e_lower and e_upper are used to compute probabilities
	feature_type e_lower[feature_count], e_upper[feature_count];
	double probabilities[feature_count];
	Node& node = nodes()[node_id];
	int const parent_id = node.parent;
	double const parent_tau = node.parent >= 0 ? nodes()[node.parent].tau : 0; //The tau value of the parent of the root is 0
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

	bool pause_expension = false;
	if((tree_management == COBBLE_MANAGEMENT || tree_management == OPTIMISTIC_COBBLE_MANAGEMENT)){
		int const remaining_depth = node_depth(node_id, nullptr);
		int const distance_to_root = unravel(node_id);
		pause_expension = remaining_depth + distance_to_root + 1 > tree_bases()[tree_id].node_count_limit;
	}
	else{
		pause_expension = tree_bases()[tree_id].is_paused(tree_management);
		//PAUSE
		//pause_expension = ((total_count >= 20 && total_count <= 150));
	}

	if(E >= 0 && parent_tau + E < node.tau && node_available >= 2 && !pause_expension){//Introduce a new parent and a new sibling
		Utils::turn_array_into_probability(probabilities, feature_count, sum);
		//sample features with probability proportional to e_lower[i] + e_upper[i]
		int const dimension = Utils::pick_from_distribution<func>(probabilities, feature_count);

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
		nodes()[new_parent].split_dimension = dimension;
		nodes()[new_parent].split_value = split_value;
		nodes()[new_parent].tau = parent_tau + E;
		//insert new leaf, sibbling of the current one
		new_sibling = available_node();
		node_available -= 2;

		//Update the box of the new parent
		for(int i = 0; i < feature_count; ++i){
			nodes()[new_parent].bound_lower[i] = features[i] < node.bound_lower[i] ? features[i] : node.bound_lower[i];
			nodes()[new_parent].bound_upper[i] = features[i] > node.bound_upper[i] ? features[i] : node.bound_upper[i];
		}
		//NOTE Creates counters for the label of the new parent
		for(int i = 0; i < label_count; ++i)
			nodes()[new_parent].counters[i] = Utils::min(1, node.counters[i]);

		//Creates counters for the label of the new sibling
		for(int i = 0; i < label_count; ++i)
			nodes()[new_sibling].counters[i] = 0;
		//No need to increase the counter for the current label because we will call sample_block soon on new_sibling

		//Make the connections between the new nodes
		nodes()[new_parent].parent = node.parent;
		if(!node.has_parent() && node_id == tree_bases()[tree_id].root)//We introduce a parent to the root
			tree_bases()[tree_id].root = new_parent;
		else{
			Node& parent = nodes()[node.parent];
			if(parent.child_left == node_id)
				parent.child_left = new_parent;
			else
				parent.child_right = new_parent;
		}


		node.parent = new_parent;

		nodes()[new_sibling].parent = new_parent;
		if(features[dimension] == upper_value){ //right
			nodes()[new_parent].child_right = new_sibling;
			nodes()[new_parent].child_left = node_id;
		}
		else{ //left
			nodes()[new_parent].child_left = new_sibling;
			nodes()[new_parent].child_right = node_id;
		}
		//cout << "Split dimension " << dimension << " on " << split_value << "(total count: " << total_count << ")" << endl;

		sample_block(new_sibling, features, label);
	}
	else{ //Otherwise, just update the box
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
				extend_block0(node.child_left, tree_id, features, label);
			else if(features[node.split_dimension] > node.split_value)
				extend_block0(node.child_right, tree_id, features, label);
			//NOTE: we don't update the counters of labels here because the counting will be done when prediction is required.
			//We can optimize that.
		}
		else{
			//Update the counter of label
			node.counters[label] += 1;
			node.fading_score += 1; //fading_score is decreased later on
		}
	}
}
//Update if not split
void extend_block2(int const node_id, int const tree_id, feature_type const* features, int const label){
	//e_lower and e_upper are used to compute probabilities
	feature_type e_lower[feature_count], e_upper[feature_count];
	double probabilities[feature_count];
	Node& node = nodes()[node_id];
	int const parent_id = node.parent;
	double const parent_tau = node.parent >= 0 ? nodes()[node.parent].tau : 0; //The tau value of the parent of the root is 0
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

	bool pause_expension = false;
	if((tree_management == COBBLE_MANAGEMENT || tree_management == OPTIMISTIC_COBBLE_MANAGEMENT)){
		int const remaining_depth = node_depth(node_id, nullptr);
		int const distance_to_root = unravel(node_id);
		pause_expension = remaining_depth + distance_to_root + 1 > tree_bases()[tree_id].node_count_limit;
	}
	else{
		pause_expension = tree_bases()[tree_id].is_paused(tree_management);
	}

	if(E >= 0 && parent_tau + E < node.tau && node_available >= 2 && !pause_expension){//Introduce a new parent and a new sibling
		Utils::turn_array_into_probability(probabilities, feature_count, sum);
		//sample features with probability proportional to e_lower[i] + e_upper[i]
		int const dimension = Utils::pick_from_distribution<func>(probabilities, feature_count);

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
		nodes()[new_parent].split_dimension = dimension;
		nodes()[new_parent].split_value = split_value;
		nodes()[new_parent].tau = parent_tau + E;
		//insert new leaf, sibbling of the current one
		new_sibling = available_node();
		node_available -= 2;

		//Update the box of the new parent
		for(int i = 0; i < feature_count; ++i){
			nodes()[new_parent].bound_lower[i] = features[i] < node.bound_lower[i] ? features[i] : node.bound_lower[i];
			nodes()[new_parent].bound_upper[i] = features[i] > node.bound_upper[i] ? features[i] : node.bound_upper[i];
		}
		//NOTE Creates counters for the label of the new parent
		//for(int i = 0; i < label_count; ++i)
		//nodes[new_parent].counters[i] = node.counters[i];
		//nodes[new_parent].counters[label] += 1;

		//Creates counters for the label of the new sibling
		for(int i = 0; i < label_count; ++i)
			nodes()[new_sibling].counters[i] = 0;
		//No need to increase the counter for the current label because we will call sample_block soon on new_sibling

		//Make the connections between the new nodes
		nodes()[new_parent].parent = node.parent;
		if(!node.has_parent() && node_id == tree_bases()[tree_id].root)//We introduce a parent to the root
			tree_bases()[tree_id].root = new_parent;
		else{
			Node& parent = nodes()[node.parent];
			if(parent.child_left == node_id)
				parent.child_left = new_parent;
			else
				parent.child_right = new_parent;
		}


		node.parent = new_parent;

		nodes()[new_sibling].parent = new_parent;
		if(features[dimension] == upper_value){ //right
			nodes()[new_parent].child_right = new_sibling;
			nodes()[new_parent].child_left = node_id;
		}
		else{ //left
			nodes()[new_parent].child_left = new_sibling;
			nodes()[new_parent].child_right = node_id;
		}

		sample_block(new_sibling, features, label);
	}
	else if(E < 0 || parent_tau + E > node.tau){ //Otherwise, just update the box
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
				extend_block2(node.child_left, tree_id, features, label);
			else if(features[node.split_dimension] > node.split_value)
				extend_block2(node.child_right, tree_id, features, label);
			//NOTE: we don't update the counters of labels here because the counting will be done when prediction is required.
			//We can optimize that.
		}
		else{
			//Update the counter of label
			node.counters[label] += 1;
			node.fading_score += 1; //fading_score is decreased later
		}
	}
}
//Ghost (stop going down when branch off but no node)
void extend_block1(int const node_id, int const tree_id, feature_type const* features, int const label){
	//e_lower and e_upper are used to compute probabilities
	feature_type e_lower[feature_count], e_upper[feature_count];
	double probabilities[feature_count];
	Node& node = nodes()[node_id];
	int const parent_id = node.parent;
	double const parent_tau = node.parent >= 0 ? nodes()[node.parent].tau : 0; //The tau value of the parent of the root is 0
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

	bool pause_expension = false;
	if((tree_management == COBBLE_MANAGEMENT || tree_management == OPTIMISTIC_COBBLE_MANAGEMENT)){
		int const remaining_depth = node_depth(node_id, nullptr);
		int const distance_to_root = unravel(node_id);
		pause_expension = remaining_depth + distance_to_root + 1 > tree_bases()[tree_id].node_count_limit;
	}
	else{
		pause_expension = tree_bases()[tree_id].is_paused(tree_management);
	}

	if(E >= 0 && parent_tau + E < node.tau && node_available >= 2 && !pause_expension){//Introduce a new parent and a new sibling
		Utils::turn_array_into_probability(probabilities, feature_count, sum);
		//sample features with probability proportional to e_lower[i] + e_upper[i]
		int const dimension = Utils::pick_from_distribution<func>(probabilities, feature_count);

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
		nodes()[new_parent].split_dimension = dimension;
		nodes()[new_parent].split_value = split_value;
		nodes()[new_parent].tau = parent_tau + E;
		//insert new leaf, sibbling of the current one
		new_sibling = available_node();
		node_available -= 2;

		//Update the box of the new parent
		for(int i = 0; i < feature_count; ++i){
			nodes()[new_parent].bound_lower[i] = features[i] < node.bound_lower[i] ? features[i] : node.bound_lower[i];
			nodes()[new_parent].bound_upper[i] = features[i] > node.bound_upper[i] ? features[i] : node.bound_upper[i];
		}
		//NOTE Creates counters for the label of the new parent
		for(int i = 0; i < label_count; ++i)
			nodes()[new_parent].counters[i] = node.counters[i];
		nodes()[new_parent].counters[label] += 1;

		//Creates counters for the label of the new sibling
		for(int i = 0; i < label_count; ++i)
			nodes()[new_sibling].counters[i] = 0;
		//No need to increase the counter for the current label because we will call sample_block soon on new_sibling

		//Make the connections between the new nodes
		nodes()[new_parent].parent = node.parent;
		if(!node.has_parent() && node_id == tree_bases()[tree_id].root)//We introduce a parent to the root
			tree_bases()[tree_id].root = new_parent;
		else{
			Node& parent = nodes()[node.parent];
			if(parent.child_left == node_id)
				parent.child_left = new_parent;
			else
				parent.child_right = new_parent;
		}


		node.parent = new_parent;

		nodes()[new_sibling].parent = new_parent;
		if(features[dimension] == upper_value){ //right
			nodes()[new_parent].child_right = new_sibling;
			nodes()[new_parent].child_left = node_id;
		}
		else{ //left
			nodes()[new_parent].child_left = new_sibling;
			nodes()[new_parent].child_right = node_id;
		}

		sample_block(new_sibling, features, label);
	}
	//else{ //Otherwise, just update the box
	else if(E < 0 || parent_tau + E > node.tau){ //Otherwise, just update the box
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
				extend_block1(node.child_left, tree_id, features, label);
			else if(features[node.split_dimension] > node.split_value)
				extend_block1(node.child_right, tree_id, features, label);
			node.counters[label] += 1;
		}
		else{
			//Update the counter of label
			node.counters[label] += 1;
			node.fading_score += 1; //fading_score is decreased later
		}
	}
	else{ //Split but no more node
		node.counters[label] += 1;
		node.fading_score += 1; //fading_score is decreased later
	}
}
//Increase counter but don't update box
void extend_block3(int const node_id, int const tree_id, feature_type const* features, int const label){
	//e_lower and e_upper are used to compute probabilities
	feature_type e_lower[feature_count], e_upper[feature_count];
	double probabilities[feature_count];
	Node& node = nodes()[node_id];
	int const parent_id = node.parent;
	double const parent_tau = node.parent >= 0 ? nodes()[node.parent].tau : 0; //The tau value of the parent of the root is 0
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

	bool pause_expension = false;
	if((tree_management == COBBLE_MANAGEMENT || tree_management == OPTIMISTIC_COBBLE_MANAGEMENT)){
		int const remaining_depth = node_depth(node_id, nullptr);
		int const distance_to_root = unravel(node_id);
		pause_expension = remaining_depth + distance_to_root + 1 > tree_bases()[tree_id].node_count_limit;
	}
	else{
		pause_expension = tree_bases()[tree_id].is_paused(tree_management);
	}

	if(E >= 0 && parent_tau + E < node.tau && node_available >= 2 && !pause_expension){//Introduce a new parent and a new sibling
		Utils::turn_array_into_probability(probabilities, feature_count, sum);
		//sample features with probability proportional to e_lower[i] + e_upper[i]
		int const dimension = Utils::pick_from_distribution<func>(probabilities, feature_count);

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
		nodes()[new_parent].split_dimension = dimension;
		nodes()[new_parent].split_value = split_value;
		nodes()[new_parent].tau = parent_tau + E;
		//insert new leaf, sibbling of the current one
		new_sibling = available_node();
		node_available -= 2;

		//Update the box of the new parent
		for(int i = 0; i < feature_count; ++i){
			nodes()[new_parent].bound_lower[i] = features[i] < node.bound_lower[i] ? features[i] : node.bound_lower[i];
			nodes()[new_parent].bound_upper[i] = features[i] > node.bound_upper[i] ? features[i] : node.bound_upper[i];
		}
		//NOTE Creates counters for the label of the new parent
		//for(int i = 0; i < label_count; ++i)
		//nodes[new_parent].counters[i] = node.counters[i];
		//nodes[new_parent].counters[label] += 1;

		//Creates counters for the label of the new sibling
		for(int i = 0; i < label_count; ++i)
			nodes()[new_sibling].counters[i] = 0;
		//No need to increase the counter for the current label because we will call sample_block soon on new_sibling

		//Make the connections between the new nodes
		nodes()[new_parent].parent = node.parent;
		if(!node.has_parent() && node_id == tree_bases()[tree_id].root)//We introduce a parent to the root
			tree_bases()[tree_id].root = new_parent;
		else{
			Node& parent = nodes()[node.parent];
			if(parent.child_left == node_id)
				parent.child_left = new_parent;
			else
				parent.child_right = new_parent;
		}


		node.parent = new_parent;

		nodes()[new_sibling].parent = new_parent;
		if(features[dimension] == upper_value){ //right
			nodes()[new_parent].child_right = new_sibling;
			nodes()[new_parent].child_left = node_id;
		}
		else{ //left
			nodes()[new_parent].child_left = new_sibling;
			nodes()[new_parent].child_right = node_id;
		}

		sample_block(new_sibling, features, label);
	}
	else if(E < 0 || parent_tau + E > node.tau){ //Otherwise, just update the box
		//update lower bound and upper bound of this node
		for(int i = 0; i < feature_count; ++i){
			if(node.bound_lower[i] > features[i])
				node.bound_lower[i] = features[i];
			if(node.bound_upper[i] < features[i])
				node.bound_upper[i] = features[i];
		}
	}
	//if not leaf, recurse on the node that contains the data point
	if(!node.is_leaf()){
		if(features[node.split_dimension] <= node.split_value)
			extend_block3(node.child_left, tree_id, features, label);
		else if(features[node.split_dimension] > node.split_value)
			extend_block3(node.child_right, tree_id, features, label);
		//NOTE: we don't update the counters of labels here because the counting will be done when prediction is required.
		//We can optimize that.
	}
	else{
		//Update the counter of label
		node.counters[label] += 1;
		node.fading_score += 1; //fading_score is decreased later
	}
}
//Count forced extend
void extend_block4(int const node_id, int const tree_id, feature_type const* features, int const label, int const forced_extend_sum=0){
	//e_lower and e_upper are used to compute probabilities
	feature_type e_lower[feature_count], e_upper[feature_count];
	double probabilities[feature_count];
	Node& node = nodes()[node_id];
	int const parent_id = node.parent;
	double const parent_tau = node.parent >= 0 ? nodes()[node.parent].tau : 0; //The tau value of the parent of the root is 0
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

	bool pause_expension = false;
	if((tree_management == COBBLE_MANAGEMENT || tree_management == OPTIMISTIC_COBBLE_MANAGEMENT)){
		int const remaining_depth = node_depth(node_id, nullptr);
		int const distance_to_root = unravel(node_id);
		pause_expension = remaining_depth + distance_to_root + 1 > tree_bases()[tree_id].node_count_limit;
	}
	else{
		pause_expension = tree_bases()[tree_id].is_paused(tree_management);
		//PAUSE
		//pause_expension = ((total_count >= 20 && total_count <= 150));
	}

	if(E >= 0 && parent_tau + E < node.tau && node_available >= 2 && !pause_expension){//Introduce a new parent and a new sibling
		bool split_instead = false;
		//If forced_extend_sum is higher than 0, then we consider spliting
		if(forced_extend_sum > 0){
			double const probability = static_cast<double>(node.forced_extend) / static_cast<double>(forced_extend_sum);
			split_instead = (func::rand_uniform() <  probability);
		}

		if(fe_split_trigger == SPLIT_TRIGGER_NONE){
			extend_node(node_id, tree_id, features, label, parent_id, parent_tau, probabilities, E, sum);
		}
		else if(fe_split_trigger == SPLIT_TRIGGER_POSITIVE && node.forced_extend > 0){
			split_node(node_id, tree_id, features, label, parent_id, parent_tau, probabilities, E);
		}
		else if((fe_split_trigger == SPLIT_TRIGGER_TOTAL || fe_split_trigger == SPLIT_TRIGGER_SFE) && split_instead){
			split_node(node_id, tree_id, features, label, parent_id, parent_tau, probabilities, E);
		}
		else{
			extend_node(node_id, tree_id, features, label, parent_id, parent_tau, probabilities, E, sum);
		}
	}
	else{ //Otherwise, just update the box
		//update lower bound and upper bound of this node
		for(int i = 0; i < feature_count; ++i){
			if(node.bound_lower[i] > features[i]){
				double const difference = (node.bound_lower[i] - features[i]) * fe_parameter;
				node.bound_lower[i] -= difference;
			}
			if(node.bound_upper[i] < features[i]){
				double const difference = (features[i] - node.bound_upper[i]) * fe_parameter;
				node.bound_upper[i] += difference;
			}
		}
		//Check if it's a forced extend
		if(E >= 0 && (parent_tau + E < node.tau)){
			node.forced_extend += 1;
		}
		//if not leaf, recurse on the node that contains the data point
		if(!node.is_leaf()){
			if(features[node.split_dimension] <= node.split_value)
				extend_block4(node.child_left, tree_id, features, label, forced_extend_sum);
			else if(features[node.split_dimension] > node.split_value)
				extend_block4(node.child_right, tree_id, features, label, forced_extend_sum);
			//NOTE: we don't update the counters of labels here because the counting will be done when prediction is required.
			//We can optimize that.
		}
		else{
			bool has_split_bary = false;
			//PAUSE
			//if(!((total_count >= 20 && total_count <= 150)) && E < 0 && node_available >= 2){//Data point in the box
			if(has_been_full > 20 && E < 0 && node_available >= 2){//Data point in the box
					has_split_bary = split_barycenter(node_id, tree_id, features, label, parent_id, parent_tau);
			}
			if(!has_split_bary){

				//Update the counter of label
				node.counters[label] += 1;
				node.fading_score += 1; //fading_score is decreased later
			}
		}
	}
}
void extend_node(int const node_id, int const tree_id, feature_type const* features, int const label, int const parent_id, double const parent_tau, double* probabilities, double const E, double const sum){
		Node& node = nodes()[node_id];
		int new_parent_id, new_sibling_id;
		bool current_node_on_left = true;
		Utils::turn_array_into_probability(probabilities, feature_count, sum);
		//sample features with probability proportional to e_lower[i] + e_upper[i]
		int const dimension = Utils::pick_from_distribution<func>(probabilities, feature_count);

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
		//insert new node above the current one
		new_parent_id = available_node();
		nodes()[new_parent_id].split_dimension = dimension;
		nodes()[new_parent_id].split_value = split_value;
		nodes()[new_parent_id].tau = parent_tau + E;
		//insert new leaf, sibbling of the current one
		new_sibling_id = available_node();
		node_available -= 2;

		//Update the box of the new parent
		for(int i = 0; i < feature_count; ++i){
			nodes()[new_parent_id].bound_lower[i] = features[i] < node.bound_lower[i] ? features[i] : node.bound_lower[i];
			nodes()[new_parent_id].bound_upper[i] = features[i] > node.bound_upper[i] ? features[i] : node.bound_upper[i];
		}
		//NOTE Creates counters for the label of the new parent
		//for(int i = 0; i < label_count; ++i)
		//nodes[new_parent_id].counters[i] = node.counters[i];
		//nodes[new_parent_id].counters[label] += 1;

		//Creates counters for the label of the new sibling
		for(int i = 0; i < label_count; ++i)
			nodes()[new_sibling_id].counters[i] = 0;
		//No need to increase the counter for the current label because we will call sample_block soon on new_sibling

		sample_block(new_sibling_id, features, label);
		if(features[dimension] == upper_value){ //right
			current_node_on_left = false;
		}
		else{
			current_node_on_left = true;
		}

		//Make the connections between the new nodes
		nodes()[new_parent_id].parent = node.parent;
		if(!node.has_parent() && node_id == tree_bases()[tree_id].root)//We introduce a parent to the root
			tree_bases()[tree_id].root = new_parent_id;
		else{
			Node& parent = nodes()[node.parent];
			if(parent.child_left == node_id)
				parent.child_left = new_parent_id;
			else
				parent.child_right = new_parent_id;
		}


		node.parent = new_parent_id;

		nodes()[new_sibling_id].parent = new_parent_id;
		if(!current_node_on_left){ //right
			nodes()[new_parent_id].child_right = new_sibling_id;
			nodes()[new_parent_id].child_left = node_id;
		}
		else{ //left
			nodes()[new_parent_id].child_left = new_sibling_id;
			nodes()[new_parent_id].child_right = node_id;
		}
}
public:
void split_node(int const node_id, int const tree_id, feature_type const* features, int const label, int const parent_id, double const parent_tau, double* probabilities, double const E){
	int new_parent_id, new_sibling_id;
	double sum = 0;
	Node& node = nodes()[node_id];
	for(int i = 0; i < feature_count; ++i){
		probabilities[i] = node.bound_upper[i] - node.bound_lower[i];
		sum += probabilities[i];
	}
	Utils::turn_array_into_probability(probabilities, feature_count, sum);
	//sample features with probability proportional to e_lower[i] + e_upper[i]
	int const dimension = Utils::pick_from_distribution<func>(probabilities, feature_count);

	//Select the bound to choose the split from
	double lower_value = node.bound_lower[dimension], upper_value = node.bound_upper[dimension];

	//sample the split between [lower_value, upper_value]
	double const random_value = func::rand_uniform();
	double const split_value = random_value*(upper_value - lower_value) + lower_value;

	new_parent_id = available_node();
	nodes()[new_parent_id].tau = parent_tau + E;
	new_sibling_id = available_node();
	nodes()[new_sibling_id].tau = lifetime;
	node_available -= 2;

	Node &new_parent = nodes()[new_parent_id];
	Node &new_sibling = nodes()[new_sibling_id];

	new_parent.split_dimension = dimension;
	new_parent.split_value = split_value;
	//Set the box for the new children and the new_parent and the node (depending on the values in features
	//The current datapoint is passed through after readjusting counters and shape
	for(int i = 0; i < feature_count; ++i){
		new_parent.bound_lower[i] = Utils::min(node.bound_lower[i], features[i]);
		new_parent.bound_upper[i] = Utils::max(node.bound_upper[i], features[i]);
		new_sibling.bound_lower[i] = node.bound_lower[i];
		new_sibling.bound_upper[i] = node.bound_upper[i];
	}

	//Creates counters for the label of the new sibling
	for(int i = 0; i < label_count; ++i)
		new_sibling.counters[i] = 0;
	//No need to increase the counter for the current label because we will do it later, when passing down.

	//Make the connections between the new nodes
	if(!node.has_parent() && node_id == tree_bases()[tree_id].root)//We introduce a parent to the root
		tree_bases()[tree_id].root = new_parent_id;
	else{ //Otherwise, replace the existing pointer to parent
		Node& parent = nodes()[node.parent];
		if(parent.child_left == node_id)
			parent.child_left = new_parent_id;
		else
			parent.child_right = new_parent_id;
	}

	//Count sub-trees !
	int count_left = 0, count_right = 0;
	count_sides(node_id, dimension, split_value, count_left, count_right);
	bool const subtree_on_left = (count_left > count_right);
	//If more count are higher than split ---> new sibling goes left
	//set the connections
	if(subtree_on_left){
		new_parent.child_right = new_sibling_id;
		new_parent.child_left = node_id;
		node.bound_upper[dimension] = split_value;
		new_sibling.bound_lower[dimension] = split_value;
	}
	else{
		new_parent.child_right = node_id;
		new_parent.child_left = new_sibling_id;
		node.bound_lower[dimension] = split_value;
		new_sibling.bound_upper[dimension] = split_value;
	}
	new_parent.parent = node.parent;
	node.parent = new_parent_id;
	new_sibling.parent = new_parent_id;

	//Distribute Forced_extend
	if(fe_distribution == FE_DISTRIBUTION_ZERO){
		node.forced_extend = 0;
		new_sibling.forced_extend = 0;
	}
	else if(fe_distribution == FE_DISTRIBUTION_SPLIT){
		new_sibling.forced_extend = 0.5 * node.forced_extend;
		node.forced_extend *= 0.5;
	}
	else if(fe_distribution == FE_DISTRIBUTION_PROPORTIONAL){
		double const sum_count = static_cast<double>(count_left + count_right);
		double const ratio_fe = subtree_on_left ? static_cast<double>(count_left)/sum_count : static_cast<double>(count_left)/sum_count;
		new_sibling.forced_extend = (1-ratio_fe) * node.forced_extend;
		node.forced_extend *= ratio_fe;
	}
	else if(fe_distribution == FE_DISTRIBUTION_DECREMENT){
		new_sibling.forced_extend = 0;
		node.forced_extend --;
	}
	new_parent.forced_extend = 0;

	int to_remove[label_count] = {0};
	//If subtree_on_left, then discharge on right
	adjust_counters(node_id, dimension, split_value, !subtree_on_left, to_remove);
	//Place the new counters in new_sibling
	for(int i = 0; i < label_count; ++i)
		new_sibling.counters[i] += to_remove[i];
	//Adjust the boxes' bound and reshape the tree is needed
	adjust_boxes(node_id, dimension, split_value, subtree_on_left, tree_id);

	//Propage the datapoint on the right side of the new split
	if(features[dimension] > split_value){
		pass_data_point_down(new_parent.child_right, features, label);
	}
	else{
		pass_data_point_down(new_parent.child_left, features, label);
	}
}
bool split_barycenter(int const node_id, int const tree_id, feature_type const* features, int const label, int const parent_id, double const parent_tau){
	int new_parent_id, new_sibling_id;
	double sum = 0;
	Node& node = nodes()[node_id];
	double barycenter[feature_count];
	double probabilities[feature_count];
	if(split_helper == SPLIT_HELPER_WEIGHTED)
		find_barycenter(tree_bases()[tree_id].root, barycenter);
	else if(split_helper == SPLIT_HELPER_AVG){
		for(int i = 0; i < feature_count; i++){
			barycenter[i] = sum_features[i]/count_points;
		}
	}
	else if(split_helper == SPLIT_HELPER_NONE)
		return false;

	for(int i = 0; i < feature_count; ++i){
		if(barycenter[i] > node.bound_lower[i] && barycenter[i] < node.bound_upper[i]){
			probabilities[i] = Utils::abs(barycenter[i] - features[i]);
			sum += probabilities[i];
		}
		else{
			probabilities[i] = 0;
		}
	}
	double const E = sum == 0 ?  -1 : Utils::rand_exponential<func>(sum);
	if(sum == 0){
		return false;
	}

	Utils::turn_array_into_probability(probabilities, feature_count, sum);
	//sample features with probability proportional to e_lower[i] + e_upper[i]
	int const dimension = Utils::pick_from_distribution<func>(probabilities, feature_count);

	//Select the bound to choose the split from
	double lower_value = Utils::min(barycenter[dimension], features[dimension]);
	double upper_value = Utils::max(barycenter[dimension], features[dimension]);

	//sample the split between [lower_value, upper_value]
	double const random_value = func::rand_uniform();
	double const split_value = random_value*(upper_value - lower_value) + lower_value;

	new_parent_id = available_node();
	nodes()[new_parent_id].tau = parent_tau + (node.tau - parent_tau)*random_value;
	new_sibling_id = available_node();
	nodes()[new_sibling_id].tau = lifetime;
	node_available -= 2;

	Node &new_parent = nodes()[new_parent_id];
	Node &new_sibling = nodes()[new_sibling_id];

	new_parent.split_dimension = dimension;
	new_parent.split_value = split_value;
	//Set the box for the new children and the new_parent and the node (depending on the values in features
	//The current datapoint is passed through after readjusting counters and shape
	for(int i = 0; i < feature_count; ++i){
		new_parent.bound_lower[i] = Utils::min(node.bound_lower[i], features[i]);
		new_parent.bound_upper[i] = Utils::max(node.bound_upper[i], features[i]);
		new_sibling.bound_lower[i] = node.bound_lower[i];
		new_sibling.bound_upper[i] = node.bound_upper[i];
	}

	//Creates counters for the label of the new sibling
	for(int i = 0; i < label_count; ++i)
		new_sibling.counters[i] = 0;
	//No need to increase the counter for the current label because we will do it later, when passing down.

	//Make the connections between the new nodes
	if(!node.has_parent() && node_id == tree_bases()[tree_id].root)//We introduce a parent to the root
		tree_bases()[tree_id].root = new_parent_id;
	else{ //Otherwise, replace the existing pointer to parent
		Node& parent = nodes()[node.parent];
		if(parent.child_left == node_id)
			parent.child_left = new_parent_id;
		else
			parent.child_right = new_parent_id;
	}

	//Count sub-trees !
	int count_left = 0, count_right = 0;
	count_sides(node_id, dimension, split_value, count_left, count_right);
	bool const subtree_on_left = (count_left > count_right);
	//If more count are higher than split ---> new sibling goes left
	//set the connections
	if(subtree_on_left){
		new_parent.child_right = new_sibling_id;
		new_parent.child_left = node_id;
		node.bound_upper[dimension] = split_value;
		new_sibling.bound_lower[dimension] = split_value;
	}
	else{
		new_parent.child_right = node_id;
		new_parent.child_left = new_sibling_id;
		node.bound_lower[dimension] = split_value;
		new_sibling.bound_upper[dimension] = split_value;
	}
	new_parent.parent = node.parent;
	node.parent = new_parent_id;
	new_sibling.parent = new_parent_id;

	//Distribute Forced_extend
	if(fe_distribution == FE_DISTRIBUTION_ZERO){
		node.forced_extend = 0;
		new_sibling.forced_extend = 0;
	}
	else if(fe_distribution == FE_DISTRIBUTION_SPLIT){
		new_sibling.forced_extend = 0.5 * node.forced_extend;
		node.forced_extend *= 0.5;
	}
	else if(fe_distribution == FE_DISTRIBUTION_PROPORTIONAL){
		double const sum_count = static_cast<double>(count_left + count_right);
		double const ratio_fe = subtree_on_left ? static_cast<double>(count_left)/sum_count : static_cast<double>(count_left)/sum_count;
		new_sibling.forced_extend = (1-ratio_fe) * node.forced_extend;
		node.forced_extend *= ratio_fe;
	}
	else if(fe_distribution == FE_DISTRIBUTION_DECREMENT){
		new_sibling.forced_extend = 0;
		node.forced_extend --;
	}
	new_parent.forced_extend = 0;

	int to_remove[label_count] = {0};
	//If subtree_on_left, then discharge on right
	adjust_counters(node_id, dimension, split_value, !subtree_on_left, to_remove);
	//Place the new counters in new_sibling
	for(int i = 0; i < label_count; ++i)
		new_sibling.counters[i] += to_remove[i];
	//Adjust the boxes' bound and reshape the tree is needed
	adjust_boxes(node_id, dimension, split_value, subtree_on_left, tree_id);

	//Propage the datapoint on the right side of the new split
	if(features[dimension] > split_value){
		pass_data_point_down(new_parent.child_right, features, label);
	}
	else{
		pass_data_point_down(new_parent.child_left, features, label);
	}
	return true;
}
template<int max_stack_size=100>
int count_sides(int const start_id, int const split_dimension, double const split_value, int& count_left, int& count_right){
	//Initialize an array to keep track of where we have to go at each tree level.
	//The max depth expected is MAX_DEPTH.
	int stack[max_stack_size];
	for(int i = 0; i < max_stack_size; ++i)
		stack[i] = -1;

	int node_id = start_id;
	int depth = 0;
	//a for loop instead of a while to avoid infinite loops. Since we don't expect to do more turn than node_count
	//*i* count the number of node deleted.
	int i = 0;
	while(i < node_count && node_id >= 0){
		Node& node = nodes()[node_id];
		if(!node.is_leaf()){  //Internal node
			if (stack[depth] == -1){ //Go right
				stack[depth] = 0;
				depth += 1;
				node_id = node.child_right;
			}
			else if (stack[depth] == 0){ //Go left
				stack[depth] = 1;
				depth += 1;
				node_id = node.child_left;
			}
			else if (stack[depth] == 1){ //Go up
				stack[depth] = -1; //Reset to -1
				depth -= 1;
				i += 1;
				if(node_id == start_id)
					break;
				node_id = node.parent;
			}
		}
		else{ //Leaf
			stack[depth] = -1; //Reset to -1
			depth -= 1;
			i += 1;

			int count = 0;
			for(int i = 0; i < label_count; ++i){
				count += node.counters[i];
			}
			double const lower = node.bound_lower[split_dimension];
			double const upper = node.bound_upper[split_dimension];
			if(split_value > lower && split_value < upper){
				double const percent = static_cast<double>(split_value-lower) / static_cast<double>(upper - lower);
				int left = static_cast<int>(static_cast<double>(count) * percent);
				int right = static_cast<int>(static_cast<double>(count) * (1-percent));
				int const err = count - left - right;
				if(err != 0 && func::rand_uniform() < 0.5)
					left += err;
				else if(err != 0)
					right += err;
				count_left += left;
				count_right += right;
			}
			else if(split_value <= lower)
				count_left += count;
			else if(split_value >= upper)
				count_right += count;

			if(node_id == start_id)
				break;
			node_id = node.parent;
		}
		if(depth >= max_stack_size)
			return -1;
	}
	if(depth != -1)
		return -1;
	return 0;
}
template<int max_stack_size=100>
int adjust_boxes(int const start_id, int const split_dimension, double const split_value, bool const subtree_on_left, int const tree_id){
	//Initialize an array to keep track of where we have to go at each tree level.
	//The max depth expected is MAX_DEPTH.
	int stack[max_stack_size];
	for(int i = 0; i < max_stack_size; ++i)
		stack[i] = -1;

	int node_id = start_id;
	int depth = 0;
	//a for loop instead of a while to avoid infinite loops. Since we don't expect to do more turn than node_count
	//*i* count the number of node deleted.
	int i = 0;
	while(i < node_count && node_id >= 0){
		Node& node = nodes()[node_id];
		if(!node.is_leaf()){  //Internal node
			if (stack[depth] == -1){ //Go right
				if(subtree_on_left){
					if(node.bound_upper[split_dimension] > split_value)
						node.bound_upper[split_dimension] = split_value;
					if(node.bound_lower[split_dimension] > split_value)
						node.bound_lower[split_dimension] = split_value;
				}
				else{
					if(node.bound_upper[split_dimension] < split_value)
						node.bound_upper[split_dimension] = split_value;
					if(node.bound_lower[split_dimension] < split_value)
						node.bound_lower[split_dimension] = split_value;
				}
				if(node.bound_upper[split_dimension] - node.bound_lower[split_dimension] == 0){ //This box is zero, let's go up
					depth -= 1;
					node_id = node.parent;
					node_reset(node.child_right);
					node_reset(node.child_left);
					//NOTE the start_id node shouldn't be at zero.
				}
				else{ //Go right
					stack[depth] = 0;
					depth += 1;
					node_id = node.child_right;
				}
			}
			else if (stack[depth] == 0){ //Go left
				stack[depth] = 1;
				depth += 1;
				node_id = node.child_left;
			}
			else if (stack[depth] == 1){ //Go up
				stack[depth] = -1; //Reset to -1
				depth -= 1;
				i += 1;

				Node const& child_left = nodes()[node.child_left];
				Node const& child_right = nodes()[node.child_right];
				int const left_zeroed = (child_left.bound_upper[split_dimension] - child_left.bound_lower[split_dimension] == 0);
				int const right_zeroed = (child_right.bound_upper[split_dimension] - child_right.bound_lower[split_dimension] == 0);
				bool const is_root = (node.parent < 0);
				bool const is_starting = (node_id == start_id);
				//NOTE either left or right is zeroed. If both are, then we stop going down at that node before
				int replacer_id = -1, other_id = -1;
				if(left_zeroed){ //replace current node with right
					replacer_id = node.child_right;
					other_id = node.child_left;
				}
				else if(right_zeroed){ //replace current node with left
					replacer_id = node.child_left;
					other_id = node.child_right;
				}

				if(replacer_id != -1){
					Node& replacer = nodes()[replacer_id];
					if(is_root){ //The root is replaced by one of the children
						tree_bases()[tree_id].root = replacer_id;
						nodes()[other_id].reset();
						node.reset();
						nodes()[replacer_id].parent = -1;
						node_available += 2;
						break;
					}
					else{ //Replace the current node in the parent 's children
						Node& parent = nodes()[node.parent];
						if(parent.child_left == node_id)
							parent.child_left = replacer_id;
						else
							parent.child_right = replacer_id;
						replacer.parent = node.parent;
					}
					//Set the id to keep exploring the tree before the reset
					node_id = node.parent;
					//reset other_id and node
					nodes()[other_id].reset();
					node.reset();
					node_available += 2;
					if(is_starting)
						break;
				}
				else{
					if(node_id == start_id)
						break;
					node_id = node.parent;
				}
			}
		}
		else{ //Leaf
			stack[depth] = -1; //Reset to -1
			depth -= 1;
			i += 1;
			if(subtree_on_left){
				if(node.bound_upper[split_dimension] > split_value)
					node.bound_upper[split_dimension] = split_value;
				if(node.bound_lower[split_dimension] > split_value)
					node.bound_lower[split_dimension] = split_value;
			}
			else{
				if(node.bound_upper[split_dimension] < split_value)
					node.bound_upper[split_dimension] = split_value;
				if(node.bound_lower[split_dimension] < split_value)
					node.bound_lower[split_dimension] = split_value;
			}
			if(node_id == start_id)
				break;
			node_id = node.parent;
		}
		if(depth >= max_stack_size)
			return -1;
	}
	if(depth != -1)
		return -1;
	return 0;
}
template<int max_stack_size=100>
int adjust_counters(int const start_id, int split_dimension, double split_value, bool discharge_left, int* to_remove){
	//Initialize an array to keep track of where we have to go at each tree level.
	//The max depth expected is MAX_DEPTH.
	int stack[max_stack_size];
	for(int i = 0; i < max_stack_size; ++i)
		stack[i] = -1;

	int node_id = start_id;
	int depth = 0;
	//a for loop instead of a while to avoid infinite loops. Since we don't expect to do more turn than node_count
	//*i* count the number of node deleted.
	int i = 0;
	while(i < node_count && node_id >= 0){
		Node& node = nodes()[node_id];
		if(!node.is_leaf()){  //Internal node
			if (stack[depth] == -1){ //Go right
				stack[depth] = 0;
				depth += 1;
				node_id = node.child_right;
			}
			else if (stack[depth] == 0){ //Go left
				stack[depth] = 1;
				depth += 1;
				node_id = node.child_left;
			}
			else if (stack[depth] == 1){ //Go up
				stack[depth] = -1; //Reset to -1
				depth -= 1;
				i += 1;
				if(node_id == start_id)
					break;
				node_id = node.parent;
			}
		}
		else{ //Leaf
			stack[depth] = -1; //Reset to -1
			depth -= 1;
			i += 1;


			//Compute size of box
			double const max_value = node.bound_upper[split_dimension] - node.bound_lower[split_dimension];
			//Get the percentage on the left (below the split value)
			double percentage = Utils::min(Utils::max((split_value - node.bound_lower[split_dimension]) / max_value, 0.0), 1.0);
			//If this sub-tree is not on the left, invert percentage
			if(!discharge_left)
				percentage = 1 - percentage;
			for(int i = 0; i < label_count; ++i){
				int counter_out = Utils::round(static_cast<double>(node.counters[i]) * percentage);
				int counter_in = Utils::round(static_cast<double>(node.counters[i]) * (1 - percentage));
				int const err = node.counters[i] - counter_in - counter_out;
				if(err != 0)
					cout << "Error counters" << endl;
				to_remove[i] += counter_out;
				node.counters[i] = counter_in;
			}
			if(node_id == start_id)
				break;
			node_id = node.parent;
		}
		if(depth >= max_stack_size)
			return -1;
	}
	if(depth != -1)
		return -1;
	return 0;
}
int pass_data_point_down(int const start_id, feature_type const* features, int const label){
	int node_id = start_id;

	//Loop to find the leaf
	while(true){
		Node& node = nodes()[node_id];
		//Update box size
		for(int i = 0; i < feature_count; ++i){
			double const lower = Utils::min(node.bound_lower[i], features[i]);
			double const upper = Utils::max(node.bound_upper[i], features[i]);
			node.bound_lower[i] = lower;
			node.bound_upper[i] = upper;
		}
		if(node.is_leaf())
			break;
		//Choose a direction
		if(features[node.split_dimension] > node.split_value)
			node_id = node.child_right;
		else
			node_id = node.child_left;
	}
	Node& node = nodes()[node_id];
	node.counters[label] += 1;
	node.fading_score += 1; //fading_score is decreased later
	return node_id;
}
int count_fe_down(int const start_id, feature_type const* features) const{
	int node_id = start_id;
	Node const* cnodes = nodes();
	int count = 0;

	//Loop to find the leaf
	while(!cnodes[node_id].is_leaf()){
		Node const&  node = cnodes[node_id];
		count += node.forced_extend;
		//Choose a direction
		if(features[node.split_dimension] > node.split_value)
			node_id = node.child_right;
		else
			node_id = node.child_left;
	}
	count += cnodes[node_id].forced_extend;
	return count;
}
template<int max_stack_size=100>
int count_dead_supply(int const start_id){
	//Initialize an array to keep track of where we have to go at each tree level.
	//The max depth expected is MAX_DEPTH.
	int stack[max_stack_size];
	for(int i = 0; i < max_stack_size; ++i)
		stack[i] = -1;

	int node_id = start_id;
	int depth = 0;
	double dead_node = 0;
	//a for loop instead of a while to avoid infinite loops. Since we don't expect to do more turn than node_count
	//*i* count the number of node deleted.
	int i = 0;
	while(i < node_count && node_id >= 0){
		Node& node = nodes()[node_id];
		if(!node.is_leaf()){  //Internal node
			if (stack[depth] == -1){ //Go right
				stack[depth] = 0;
				depth += 1;
				node_id = node.child_right;
			}
			else if (stack[depth] == 0){ //Go left
				stack[depth] = 1;
				depth += 1;
				node_id = node.child_left;
			}
			else if (stack[depth] == 1){ //Go up
				stack[depth] = -1; //Reset to -1
				depth -= 1;
				i += 1;
				if(node_id == start_id)
					break;
				node_id = node.parent;
			}
		}
		else{ //Leaf
			stack[depth] = -1; //Reset to -1
			depth -= 1;
			i += 1;

			//Count the number of data points
			int count = 0;
			for(int i = 0; i < label_count; ++i)
				count += node.counters[i];
			//Check if it's a dead node
			if (count < 50)
				dead_node += 1;

			if(node_id == start_id)
				break;
			node_id = node.parent;
		}
		if(depth >= max_stack_size)
			return -1;
	}
	if(depth != -1)
		return -1;
	return dead_node;
}
double count_dead_supply_tree(int const tree_id){
	int const root = tree_bases()[tree_id].root;
	if(root < 0)
		return 0;
	return count_dead_supply(root);
}
template<int max_stack_size=100>
int count_data_point_node(int const start_id){
	//Initialize an array to keep track of where we have to go at each tree level.
	//The max depth expected is MAX_DEPTH.
	int stack[max_stack_size];
	for(int i = 0; i < max_stack_size; ++i)
		stack[i] = -1;

	int node_id = start_id;
	int depth = 0;
	double sum = 0;
	//a for loop instead of a while to avoid infinite loops. Since we don't expect to do more turn than node_count
	//*i* count the number of node deleted.
	int i = 0;
	while(i < node_count && node_id >= 0){
		Node& node = nodes()[node_id];
		if(!node.is_leaf()){  //Internal node
			if (stack[depth] == -1){ //Go right
				stack[depth] = 0;
				depth += 1;
				node_id = node.child_right;
			}
			else if (stack[depth] == 0){ //Go left
				stack[depth] = 1;
				depth += 1;
				node_id = node.child_left;
			}
			else if (stack[depth] == 1){ //Go up
				stack[depth] = -1; //Reset to -1
				depth -= 1;
				i += 1;
				if(node_id == start_id)
					break;
				node_id = node.parent;
			}
		}
		else{ //Leaf
			stack[depth] = -1; //Reset to -1
			depth -= 1;
			i += 1;

			//Count the number of data points
			for(int i = 0; i < label_count; ++i)
				sum += node.counters[i];

			if(node_id == start_id)
				break;
			node_id = node.parent;
		}
		if(depth >= max_stack_size)
			return -1;
	}
	if(depth != -1)
		return -1;
	return sum;
}
int count_data_point_tree(int const tree_id){
	int const root = tree_bases()[tree_id].root;
	if(root < 0)
		return 0;
	return count_data_point_node(root);
}
template<int max_stack_size=100>
int find_barycenter(int const start_id, double* avg_features){
	//Initialize an array to keep track of where we have to go at each tree level.
	//The max depth expected is MAX_DEPTH.
	int stack[max_stack_size];
	for(int i = 0; i < max_stack_size; ++i)
		stack[i] = -1;

	int node_id = start_id;
	int depth = 0;
	double count[feature_count];
	for(int i = 0; i < feature_count; ++i){
		count[i] = 0;
		avg_features[i] = 0;
	}
	//a for loop instead of a while to avoid infinite loops. Since we don't expect to do more turn than node_count
	//*i* count the number of node deleted.
	int i = 0;
	while(i < node_count && node_id >= 0){
		Node& node = nodes()[node_id];
		if(!node.is_leaf()){  //Internal node
			if (stack[depth] == -1){ //Go right
				stack[depth] = 0;
				depth += 1;
				node_id = node.child_right;
			}
			else if (stack[depth] == 0){ //Go left
				stack[depth] = 1;
				depth += 1;
				node_id = node.child_left;
			}
			else if (stack[depth] == 1){ //Go up
				stack[depth] = -1; //Reset to -1
				depth -= 1;
				i += 1;
				if(node_id == start_id)
					break;
				node_id = node.parent;
			}
		}
		else{ //Leaf
			stack[depth] = -1; //Reset to -1
			depth -= 1;
			i += 1;

			//Count the number of data points
			double data_point_count = 0;
			for(int i = 0; i < label_count; ++i)
				data_point_count += static_cast<double>(node.counters[i]);
			for(int i = 0; i < feature_count; ++i){
				double const middle = (node.bound_upper[i] + node.bound_lower[i]) * 0.5;
				avg_features[i] += middle * data_point_count;
				count[i] += data_point_count;
			}

			if(node_id == start_id)
				break;
			node_id = node.parent;
		}
		if(depth >= max_stack_size)
			return -1;
	}
	if(depth != -1)
		return -1;
	for(int i = 0; i < feature_count; ++i){
		avg_features[i] /= count[i];
	}
	return 0;
}
void tree_trim(){
	if(trim_type == TRIM_NONE)
		return;
	for(int i = 0; i < tree_count; ++i){
		int id = tree_trim(i);
		if(tree_bases()[i].root != id && id >= 0 && id < node_count) //Can't cut the root
			cut_block(id, i);
	}
}
template<int max_stack_size=100>
int tree_trim(int const tree_id){
	TreeBase const& base = tree_bases()[tree_id];
	int const root_id = base.root;

	//Initialize an array to keep track of where we have to go at each tree level.
	//The max depth expected is MAX_DEPTH.
	int stack[max_stack_size];
	for(int i = 0; i < max_stack_size; ++i)
		stack[i] = -1;

	//Start at the root, then dive inside the tree
	int node_id = root_id;
	int depth = 0;
	double smallest_count = -1;
	int smallest_id = 0;
	//a for loop instead of a while to avoid infinite loops. Since we don't expect to do more turn than node_count
	int i = 0;
	double leaf_count = 0;
	double sum_count_leaves = 0;
	while(i < node_count && node_id >= 0){
		Node& node = nodes()[node_id];
		if(!node.is_leaf()){  //Internal node
			if (stack[depth] == -1){ //Go right
				stack[depth] = 0;
				depth += 1;
				node_id = node.child_right;
			}
			else if (stack[depth] == 0){ //Go left
				stack[depth] = 1;
				depth += 1;
				node_id = node.child_left;
			}
			else if (stack[depth] == 1){ //Go up
				stack[depth] = -1; //Reset to -1
				depth -= 1;
				i += 1;
				node_id = node.parent;
			}
		}
		else{ //Leaf
			stack[depth] = -1; //Reset to -1
			depth -= 1;
			i += 1;
			leaf_count += 1;
			if(trim_type == TRIM_COUNT){//Trim based on count
				double count = 0;
				for(int i = 0; i < label_count; ++i)
					count += static_cast<double>(node.counters[i]);
				sum_count_leaves += count;
				if(smallest_count < 0 || count < smallest_count || (count == smallest_count && func::rand_uniform() < 0.5)){
					smallest_count = count;
					smallest_id = node_id;
				}
			}
			else if(trim_type == TRIM_FADING){//Trim based on a fading count
				sum_count_leaves += node.fading_score;
				if(smallest_count < 0 || node.fading_score < smallest_count || (node.fading_score == smallest_count && func::rand_uniform() < 0.5)){
					smallest_count = node.fading_score;
					smallest_id = node_id;
				}
			}
			else if(trim_type == TRIM_RANDOM){//Trim based on Reservoir Sampling
				double const r = func::rand_uniform();

				if(r < (1.0/leaf_count))
					smallest_id = node_id;
			}
			node_id = node.parent;
		}
		if(depth >= max_stack_size)
			return -1;
	}
	if(depth != -1)
		return -2;
	if(sum_count_leaves > 0 && (smallest_count/sum_count_leaves) > maximum_trim_size)
		return -3;
	return smallest_id;
}
void tree_fade_counts(){
	for(int i = 0; i < tree_count; ++i)
		tree_fade_counts(i);
}
template<int max_stack_size=100>
int tree_fade_counts(int const tree_id){
	TreeBase const& base = tree_bases()[tree_id];
	int const root_id = base.root;

	//Initialize an array to keep track of where we have to go at each tree level.
	//The max depth expected is MAX_DEPTH.
	int stack[max_stack_size];
	for(int i = 0; i < max_stack_size; ++i)
		stack[i] = -1;

	//Start at the root, then dive inside the tree
	int node_id = root_id;
	int depth = 0;
	double smallest_count = -1;
	int smallest_id = 0;
	//a for loop instead of a while to avoid infinite loops. Since we don't expect to do more turn than node_count
	int i = 0;
	double leaf_count = 0;
	double sum_count_leaves = 0;
	while(i < node_count && node_id >= 0){
		Node& node = nodes()[node_id];
		if(!node.is_leaf()){  //Internal node
			if (stack[depth] == -1){ //Go right
				stack[depth] = 0;
				depth += 1;
				node_id = node.child_right;
			}
			else if (stack[depth] == 0){ //Go left
				stack[depth] = 1;
				depth += 1;
				node_id = node.child_left;
			}
			else if (stack[depth] == 1){ //Go up
				stack[depth] = -1; //Reset to -1
				depth -= 1;
				i += 1;
				node_id = node.parent;
			}
		}
		else{ //Leaf
			stack[depth] = -1; //Reset to -1
			depth -= 1;
			i += 1;
			node.fading_score = node.fading_score * nodes_fading_f;
			node_id = node.parent;
		}
		if(depth >= max_stack_size)
			return -1;
	}
	return 0;
}
private:
void cut_block(int const node_id, int const tree_id){
	Node& node = nodes()[node_id];
	Node& parent = nodes()[node.parent];
	int sibling_id;
	if(parent.child_right == node_id)
		sibling_id = parent.child_left;
	else
		sibling_id = parent.child_right;
	Node& sibling = nodes()[sibling_id];

	if(parent.parent == -1){ //Place the other sibling as root (behead)
		parent.reset();
		node_available += 1;
		node_reset(node_id);
		//Cut off node and everything below
		tree_bases()[tree_id].root = sibling_id;
		sibling.parent = Node::EMPTY_NODE;
		return;
	}
	Node& grandparent = nodes()[parent.parent];
	int rem_count_id = parent.parent;
	if(grandparent.child_right == node.parent)
		grandparent.child_right = sibling_id;
	else
		grandparent.child_left = sibling_id;
	sibling.parent = parent.parent;
	node_available += 1;
	parent.reset();
	//TODO remove the counters?
	node_reset(node_id);
}
void split_leaf(int const node_id, int const tree_id, bool const proportianal = false){
	if(node_available < 2)
		return;
	Node& node = nodes()[node_id];
	double probabilities[feature_count];
	double sum = 0;
	for(int i = 0; i < feature_count; ++i){
		probabilities[i] = node.bound_upper[i] - node.bound_lower[i];
		sum += probabilities[i];
	}
	Utils::turn_array_into_probability(probabilities, feature_count, sum);
	//sample features with probability proportional to e_lower[i] + e_upper[i]
	int const dimension = Utils::pick_from_distribution<func>(probabilities, feature_count);

	//Select the bound to choose the split from
	double lower_value = node.bound_lower[dimension], upper_value = node.bound_upper[dimension];

	//sample the split between [lower_value, upper_value]
	double const random_value = func::rand_uniform();
	double const split_value = random_value*(upper_value - lower_value) + lower_value;
	node.split_dimension = dimension;
	node.split_value = split_value;

	double const E_right = func::rand_uniform()*(lifetime-node.tau);
	double const E_left = func::rand_uniform()*(lifetime-node.tau);
	int cright, cleft;
	cright = available_node();
	nodes()[cright].tau = node.tau + E_right;
	cleft = available_node();
	nodes()[cleft].tau = node.tau + E_left;
	node_available -= 2;

	Node &righty = nodes()[cright];
	Node &lefty = nodes()[cleft];

	//Set the box for the new children
	for(int i = 0; i < feature_count; ++i){
		righty.bound_lower[i] = lefty.bound_lower[i] = node.bound_lower[i];
		righty.bound_upper[i] = lefty.bound_upper[i] = node.bound_upper[i];
	}
	lefty.bound_upper[dimension] = split_value;
	righty.bound_lower[dimension] = split_value;
	//NOTE no need to propagate since we only split on leaves

	//NOTE Creates counters for the label of the new parent
	//for(int i = 0; i < label_count; ++i)
	//nodes[new_parent].counters[i] = node.counters[i];
	//nodes[new_parent].counters[label] += 1;

	//Creates counters for the label of the new sibling
	double split_proportions_a = proportianal ? 0.5 : random_value;
	double split_proportions_b = 1 - split_proportions_a;
	for(int i = 0; i < label_count; ++i){

		lefty.counters[i] = static_cast<int>(static_cast<double>(node.counters[i]) * split_proportions_a);
		righty.counters[i] = static_cast<int>(static_cast<double>(node.counters[i]) * split_proportions_b);
		if(lefty.counters[i] + righty.counters[i] != node.counters[i]){
			int err = Utils::abs(lefty.counters[i] + righty.counters[i] - node.counters[i]);
			if(func::rand_uniform() < 0.5)
				lefty.counters[i] += err;
			else
				righty.counters[i] += err;
		}
	}
	lefty.parent = node_id;
	righty.parent = node_id;
	node.child_right = cright;
	node.child_left = cleft;

}
/**
 * Given a node, apply the sample algorithm described in the Mondrian paper..
 * @param node_id The index of the node in the array *nodes*.
 * @param features The features of the new data point.
 * @param label The label of the new data  point.
 */
void sample_block(int const node_id, feature_type const* features, int const label){
	Node& node = nodes()[node_id];
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
	int root_id;
	TreeBase& base = tree_bases()[tree_id];
	bool ret = true;
#ifdef DEBUG
	cout << "Training Tree " << tree_id << endl;
#endif
	if (base.is_empty()){ //The root of the tree does not exist yet
		//Pick a new node
		root_id = available_node();
		if(root_id >= 0){
			//Initialize this node as the root for this tree
			tree_bases()[tree_id].root = root_id;
			Node& root = nodes()[root_id];
			///TODO to change in order to separate node from forest
			root.parent = -1;
			root.child_right = -1;
			root.child_left = -1;
			root.tau = 0;

			node_available -= 1;

			//Sample the root with the new data point
			sample_block(root_id, features, label);
			base.statistics.increase_error();
		}
		else{
			ret = false;
		}
	}
	else{ //Partial fit
		root_id = base.root;
		//add the sum of forced_extend, or the total_count
		int summy = 0;
		if(fe_split_trigger == SPLIT_TRIGGER_TOTAL)
			summy = total_count;
		else if(fe_split_trigger == SPLIT_TRIGGER_SFE)
			summy = count_fe_down(base.root, features);

		if(extend_type == EXTEND_ORIGINAL)
			extend_block0(root_id, tree_id, features, label);
		else if(extend_type == EXTEND_NONE && node_available <= 2)
			ret = false;
		else if(extend_type == EXTEND_PARTIAL_UPDATE)
			extend_block2(root_id, tree_id, features, label);
		else if(extend_type == EXTEND_GHOST)
			extend_block1(root_id, tree_id, features, label);
		else if(extend_type == EXTEND_COUNTER_NO_UPDATE)
			extend_block3(root_id, tree_id, features, label);
		else if(extend_type == EXTEND_BARYCENTER)
			extend_block4(root_id, tree_id, features, label, summy);
	}
	return ret;
}
/**
 * Compute the posterior mean at a node based on the posterior mean of its parent.
 * @param node A reference of the node.
 * @param posterior_mean The posterior mean of the parent. It will be turned into the posterior mean of the node.
 */
void compute_posterior_mean(Node const& node, double *posterior_mean) const{
	//posterior_mean start with the parent posterior_mean and the function updates to the current node
	double const parent_tau = node.parent >= 0 ? nodes()[node.parent].tau : 0; //The tau value of the parent of the root is 0
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
void predict_tree(feature_type const* features, int const tree_id, double* posterior_means, int const depth_limit = -1) const{
	int node_id = tree_bases()[tree_id].root;
	int depth = 0;
	for(int i = 0; i < label_count; ++i)
		posterior_means[i] = base_measure;

	if(node_id == -1) //The tree is empty. just return.
		return;
	double pp_sum = 0, pp_mul = 1;
	double probability_of_branching;
	double probability_not_separated_yet = 1;
	double parent_tau = 0;

	double smoothed_posterior_means[label_count] = {0};

	//Find the corresponding leaf for the data point
	while(node_id >= 0 && node_id < node_count) {
		Node const& current_node = nodes()[node_id];

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
		bool too_deep = (depth+1 >= depth_limit) && (depth_limit >= 0);
		if(too_deep || current_node.is_leaf()){
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
		depth += 1;
	}
}
int get_leaf(int const tree_id, feature_type const* features) const{
	int node_id = tree_bases()[tree_id].root;
	if(node_id < 0)
		return Node::EMPTY_NODE;

	//Loop to find the leaf
	while(true){
		Node const& node = nodes()[node_id];
		if(node.is_leaf())
			return node_id;
		//Choose a direction
		if(features[node.split_dimension] > node.split_value)
			node_id = node.child_right;
		else
			node_id = node.child_left;
	}
	return Node::EMPTY_NODE;
}
/**
 * Update the label counters of an internal node based on the last data point.
 * Faster to run because it only update the branch of the last data point.
 * @param node_id The id of the node to update.
 */
void update_posterior_count(int const tree_id, feature_type const* features, int const label){
	int node_id = get_leaf(tree_id, features);
	int const leaf_id = node_id;
	node_id = nodes()[node_id].parent;
	while(node_id > 0 && node_id < node_count){
		Node& node = nodes()[node_id];
		int const c_left = Utils::min(1, nodes()[node.child_left].counters[label]);
		int const c_right = Utils::min(1, nodes()[node.child_right].counters[label]);
		node.counters[label] = c_left + c_right;
		//TODO check validity of that if
		//if(node.counters[i] == 2)
			//break;
		node_id = node.parent;
	}
}
/**
 * Update the label counters of an internal node.
 * It is a recursive function so if apply to the root, it will apply to an entire tree.
 * @param node_id The id of the node to update.
 */
void update_posterior_count(int const node_id){
	Node& node = nodes()[node_id];
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
		int const c_left = Utils::min(1, nodes()[node.child_left].counters[i]);
		int const c_right = Utils::min(1, nodes()[node.child_right].counters[i]);
		node.counters[i] = c_left + c_right;
	}
}
/**
 * Update the label counters of all trees.
 */
void update_posterior_count(void){
	//For each tree, run the recursive *update_posterior_count* on the root
	TreeBase* bases = tree_bases();
	for(int i = 0; i < tree_count; ++i)
		if(!bases[i].is_empty())
			update_posterior_count(bases[i].root);
}
double average_tree_size(void) const{
	double sum = 0;
	TreeBase const* tb = tree_bases();
	for(int i = 0; i < tree_count; ++i)
		sum += tb[i].size;
	return sum/static_cast<double>(tree_count);
}
/**
 * Compute the depth of a tree. The template max_stack_size is the maximum depth expected.
 * @param tree_id The id of the tree which is an index between 0 and tree_count.
 */
template<int max_stack_size=100>
int tree_depth(int const tree_id, int* tree_node_count) const{
	int root_id;
	TreeBase const& base = tree_bases()[tree_id];
	root_id = base.root;

	if(root_id < 0){
		if(tree_node_count != nullptr)
			*tree_node_count = 0;
		return 0;
	}

	return node_depth<max_stack_size>(root_id, tree_node_count);
}
template<int max_stack_size=100>
int node_depth(int const start_id, int* tree_node_count) const{
	//Initialize an array to keep track of where we have to go at each tree level.
	//The max depth expected is MAX_DEPTH.
	int stack[max_stack_size];
	for(int i = 0; i < max_stack_size; ++i)
		stack[i] = -1;

	int node_id = start_id;
	int depth = 0, max_depth = 0;

	//a for loop instead of a while to avoid infinite loops. Since we don't expect to do more turn than node_count
	//*i* count the number of node deleted.
	int i = 0;
	int depth_id = start_id;
	while(i < node_count && depth >= 0){
		Node const& node = nodes()[node_id];
		if (node.is_leaf()){
			i += 1;
			//acknowledge the depth
			if(depth > max_depth){
				max_depth = depth;
				depth_id = node_id;
			}

			node_id = node.parent;
			depth -= 1;
		}
		else{ //Internal Node
			if (stack[depth] == -1){ //going right
				stack[depth] = 0;
				depth += 1;
				node_id = node.child_right;
			}
			else if (stack[depth] == 0){ //Go left
				stack[depth] = 1;
				depth += 1;
				node_id = node.child_left;
			}
			else if (stack[depth] == 1){ //Go up
				i += 1;
				stack[depth] = -1; //Reset to -1
				depth -= 1;
				node_id = node.parent;
			}
			#ifdef DEBUG
			else{
				cout << __FILE__ << ":" << __LINE__ << " CoarseMondrianForest::node_depth: stack[" << depth << "] == " << stack[depth] << " (should be -1, 0, or 1)" << endl;
			}
			#endif
		}
	}
	if(tree_node_count != nullptr)
		*tree_node_count = i;
	return max_depth+1;
}
/**
 * Reset all node of the tree but the root. The template max_stack_size is the maximum depth expected.
 * @param tree_id The id of the tree which is an index between 0 and tree_count.
 * @return Returns false if the maximum depth is exceed or if the tree contains more than *node_count*, which is an error in the structure.
 */
template<int max_stack_size=100>
bool tree_reset(int const tree_id) {
	TreeBase const& base = tree_bases()[tree_id];
	int const root_id = base.root;

	bool ret = node_reset<max_stack_size>(root_id);
	//Reclaim the root after reset all nodes!
	if(tree_management == ROBUR_MANAGEMENT)
		tree_bases()[tree_id].reset(size_limit); //The size_limit has been corrected in constructor if needed.
	else if(tree_management == COBBLE_MANAGEMENT || tree_management == OPTIMISTIC_COBBLE_MANAGEMENT)
		tree_bases()[tree_id].reset(size_limit);
	else
		tree_bases()[tree_id].reset(node_count);

	return ret;
}
template<int max_stack_size=100>
bool node_reset(int const start_id) {
	int const root_id = start_id;
	//Initialize an array to keep track of where we have to go at each tree level.
	//The max depth expected is MAX_DEPTH.
	int stack[max_stack_size];
	for(int i = 0; i < max_stack_size; ++i)
		stack[i] = -1;

	//Start at the root, then dive inside the tree
	int node_id = root_id;
	int depth = 0;
	//a for loop instead of a while to avoid infinite loops. Since we don't expect to do more turn than node_count
	//*i* count the number of node deleted.
	int i = 0;
	while(i < node_count && node_id >= 0){
		Node& node = nodes()[node_id];
		if (node.is_leaf()){
			node_available += 1;
			i += 1;

			if(node_id == root_id){
				node.reset();
				break;
			}

			//Get the parent before reset (root parent will be a negative number)
			//Since the loop stops if node_id is below 0, it's fine
			node_id = node.parent;
			node.reset();

			depth -= 1;
		}
		else{  //Internal node
			if (stack[depth] == -1){ //going right
				stack[depth] = 0;
				depth += 1;
				node_id = node.child_right;
			}
			else if (stack[depth] == 0){ //Go left
				stack[depth] = 1;
				depth += 1;
				node_id = node.child_left;
			}
			else if (stack[depth] == 1){ //Go up
				node_available += 1;

				if(node_id == root_id){
					node.reset();
					break;
				}

				stack[depth] = -1; //Reset to -1
				depth -= 1;
				i += 1;

				//Get the parent before reset (root parent will be a negative number)
				//Since the loop stops if node_id is below 0, it's fine
				node_id = node.parent;
				node.reset();

			}
		}
		if(depth >= max_stack_size)
			return false;
	}

	if(depth != -1)
		return false;
	return true;
}
void relocate_node(int const old_index, int const new_index){
	Node* n = nodes();
	if(n[old_index].available()){
#ifdef DEBUG
		cout << "Relocate of " << old_index << " not done because node is empty." << endl;
#endif
		return;
	}
	else{
#ifdef DEBUG
		cout << "Relocate " << old_index << " to " << new_index << endl;
#endif
	}
	n[new_index] = n[old_index];
	Node& node = n[new_index];
	if(node.parent != Node::EMPTY_NODE){ //Change our parent's pointer if needed.
		Node& parent = n[node.parent];
		if(parent.child_left == old_index)
			parent.child_left = new_index;
		else
			parent.child_right = new_index;
	}
	else{ //Otherwise, change the root
		TreeBase* bases = tree_bases();
		for(int i = 0; i < tree_count; ++i)
			if(bases[i].root == old_index){
				bases[i].root = new_index;
			}
	}
	if(node.child_left != Node::EMPTY_NODE){ //Change children's parent if needed
		n[node.child_left].parent = new_index;
		n[node.child_right].parent = new_index;
	}
}
bool tree_add(void){
	int const index_tree_base = max_size - tree_count * sizeof(TreeBase);
	int const new_index_tree_base = index_tree_base - sizeof(TreeBase);
	int const index_last_node = node_count * sizeof(Node);

	if(new_index_tree_base < index_last_node){ //We need to relocate nodes
		int const space_to_free = index_last_node - new_index_tree_base;
		int const number_of_node_to_move = (space_to_free - space_to_free%sizeof(Node)) / sizeof(Node) + ((space_to_free%sizeof(Node)) > 0);
		int const old_node_count = node_count;
		if(2*number_of_node_to_move > node_available) //2 time because we gonna remove *number_of_node_to_move* from the pool and we need *number_of_node_to_move* additional node to relocate them.
			return false;
		//Update node_count since *available_node* works with that number, that's perfect
		node_count -= number_of_node_to_move;
		//We remove exactly *number_of_node_to_move* because if the node to move is empty, there is one less node available.
		//If the node is node empty, we need to relocate it to an available node, therefore minus 1 for node_available.
		node_available -= number_of_node_to_move;
		for(int i = 0; i < number_of_node_to_move; ++i){
			int const relocating_index = available_node();
			relocate_node(node_count + i, relocating_index);
		}
	}
	//Shift all the tree base (to keep order of the tree :D. Mainly to track the performance of each tree, otherwise, we can just update tree_count and reset base 0.
	for(int i = 0; i < tree_count; ++i)
		tree_bases()[i-1] = tree_bases()[i];
	tree_count += 1;

	//initialize the new tree base;
	tree_bases()[tree_count-1].reset();
	return true;
}
bool tree_delete(int const tree_id){
	tree_reset(tree_id);
	//From here, the tree *tree_id* should only have the root active.

	int const index_tree_base = max_size - tree_count * sizeof(TreeBase);
	int const new_index_tree_base = index_tree_base + sizeof(TreeBase);
	int const index_last_node = node_count * sizeof(Node);


	//int const root_id = tree_bases()[tree_id].root;
	//Node& root = nodes()[root_id];
	//root.reset();

	TreeBase* bases = tree_bases();
	for(int tid = tree_id; tid > 0; --tid){
		bases[tid] = bases[tid-1];
	}
	int const freed_byte = new_index_tree_base - index_last_node;
	int const freed_node = (freed_byte - (freed_byte%sizeof(Node)))/sizeof(Node);
	node_count += freed_node;
	tree_count -= 1;

	return true;
}
template<int max_stack_size=100>
bool tree_chop(int const tree_id, int const cutting_depth = -1) { //Chop every depth below *cutting_depth*
	TreeBase const& base = tree_bases()[tree_id];
	int const root_id = base.root;

	//Initialize an array to keep track of where we have to go at each tree level.
	//The max depth expected is MAX_DEPTH.
	int stack[max_stack_size];
	for(int i = 0; i < max_stack_size; ++i)
		stack[i] = -1;

	//Start at the root, then dive inside the tree
	int node_id = root_id;
	int depth = 0;
	//a for loop instead of a while to avoid infinite loops. Since we don't expect to do more turn than node_count
	//*i* count the number of node deleted.
	int i = 0;
	while(i < node_count && node_id >= 0){
		Node& node = nodes()[node_id];
		if(!node.is_leaf()){  //Internal node
			if (stack[depth] == -1){ //Go right or cut
				if((depth+1) == cutting_depth){
					//Chop chop
					node_reset(node.child_right);
					node_reset(node.child_left);
					node.chop();

					//Go up
					node_id = node.parent;
					depth -= 1;
				}
				else{ //Go right
					stack[depth] = 0;
					depth += 1;
					node_id = node.child_right;
				}
			}
			else if (stack[depth] == 0){ //Go left
				stack[depth] = 1;
				depth += 1;
				node_id = node.child_left;
			}
			else if (stack[depth] == 1){ //Go up
				stack[depth] = -1; //Reset to -1
				depth -= 1;
				i += 1;
				node_id = node.parent;
			}
		}
		else{ //Leaf
			stack[depth] = -1; //Reset to -1
			depth -= 1;
			i += 1;
			node_id = node.parent;
		}
		if(depth >= max_stack_size)
			return false;
	}
	if(depth != -1)
		return false;
	return true;
}
bool trees_chop(){
	bool ret = true;
	for(int i = 0; i < tree_count; ++i)
		ret = ret & tree_chop(i);
	return ret;
}
void tree_reshape(){
	tree_trim();
	tree_split();
}
void tree_split(){
	for(int i = 0; i < tree_count; ++i){
		int id = tree_split(i);
		split_leaf(id, i, false);
	}
}
template<int max_stack_size=100>
int tree_split(int const tree_id){
	TreeBase const& base = tree_bases()[tree_id];
	int const root_id = base.root;

	//Initialize an array to keep track of where we have to go at each tree level.
	//The max depth expected is MAX_DEPTH.
	int stack[max_stack_size];
	for(int i = 0; i < max_stack_size; ++i)
		stack[i] = -1;

	//Start at the root, then dive inside the tree
	int node_id = root_id;
	int depth = 0;
	int biggest_count = 0, biggest_id = 0;
	//a for loop instead of a while to avoid infinite loops. Since we don't expect to do more turn than node_count
	//*i* count the number of node deleted.
	int i = 0;
	while(i < node_count && node_id >= 0){
		Node& node = nodes()[node_id];
		if(!node.is_leaf()){  //Internal node
			if (stack[depth] == -1){ //Go right
				stack[depth] = 0;
				depth += 1;
				node_id = node.child_right;
			}
			else if (stack[depth] == 0){ //Go left
				stack[depth] = 1;
				depth += 1;
				node_id = node.child_left;
			}
			else if (stack[depth] == 1){ //Go up
				stack[depth] = -1; //Reset to -1
				depth -= 1;
				i += 1;
				node_id = node.parent;
			}
		}
		else{ //Leaf
			stack[depth] = -1; //Reset to -1
			depth -= 1;
			i += 1;
			int count = node.forced_extend;
			if(count > biggest_count || (count == biggest_count && func::rand_uniform() < 0.5)){
				biggest_count = count;
				biggest_id = node_id;
			}
			node_id = node.parent;
		}
		if(depth >= max_stack_size)
			return -1;
	}
	if(depth != -1)
		return -1;
	return biggest_id;
}
template<bool verbose=false>
int unravel(int const node_id){

	if(node_id >= node_count){
 		#ifdef DEBUG
		cout << "WARNING: " << node_id << " is higher than node_count (" << node_count << ") (" << __FILE__ << ":" << __LINE__ << ")" << endl;
		#endif
		return -1;
	}
	int cur = node_id;
	int count = 0;
	while(nodes()[cur].parent != -1){
 		#ifdef DEBUG
		if(verbose)
			cout << "Unravelling " << cur << endl;
		#endif
		cur = nodes()[cur].parent;
		++count;
	}
	#ifdef DEBUG
	if(verbose)
		cout << "Root: " << cur << endl;
	int tree_id = -1;
	for(int i = 0; i < tree_count; ++i){
		if(tree_bases()[i].root == cur){
			if(verbose)
				cout << "Root of " << i << endl;
			tree_id = i;
		}
	}
	if(verbose && tree_id == -1)
		cout << "Root of none of the " << tree_count << " trees.";
	#endif
	return count;
}
#ifdef DEBUG
void child_of(int const node_id){
	for(int i = 0; i < node_count; ++i){
		if(!nodes()[i].available() && nodes()[i].parent == node_id)
			cout << "Node " << i << " has " << nodes()[i].parent << " as a parent " << (nodes()[i].parent == node_id) << endl;
	}
}
int tree_dd(int const tree_id) const{
	int const max_stack_size=100;
	int root_id;
	TreeBase const& base = tree_bases()[tree_id];
	root_id = base.root;

	//Initialize an array to keep track of where we have to go at each tree level.
	//The max depth expected is MAX_DEPTH.
	int stack[max_stack_size];
	for(int i = 0; i < max_stack_size; ++i)
		stack[i] = -1;

	int node_id = root_id;
	int depth = 0, max_depth = 1;

	//a for loop instead of a while to avoid infinite loops. Since we don't expect to do more turn than node_count
	//*i* count the number of node deleted.
	int i = 0;
	while(i < node_count && node_id >= 0){
		Node const& node = nodes()[node_id];
		if (node.is_leaf()){
			i += 1;
			//acknowledge the depth
			if(depth > max_depth)
				max_depth = depth;

			cout << "DD: " << node_id << " (leaf)" << endl;
			node_id = node.parent;
			depth -= 1;
		}
		else{ //Internal Node
			if (stack[depth] == -1){ //going right
				stack[depth] = 0;
				depth += 1;
				cout << "DD: " << node_id << " (right)" << endl;
				node_id = node.child_right;
			}
			else if (stack[depth] == 0){ //Go left
				stack[depth] = 1;
				depth += 1;
				cout << "DD: " << node_id << " (left)" << endl;
				node_id = node.child_left;
			}
			else if (stack[depth] == 1){ //Go up
				i += 1;
				stack[depth] = -1; //Reset to -1
				depth -= 1;
				cout << "DD: " << node_id << " (up)" << endl;
				node_id = node.parent;
			}
			#ifdef DEBUG
			else{
				cout << __FILE__ << ":" << __LINE__ << " CoarseMondrianForest::tree_depth: stack[" << depth << "] == " << stack[depth] << " (should be -1, 0, or 1)" << endl;
				return max_depth+1;
			}
			#endif
		}
	}
	return max_depth+1;
}
int tree_checker(int const tree_id) const{
	int const max_stack_size=100;
	int root_id;
	TreeBase const& base = tree_bases()[tree_id];
	root_id = base.root;

	//Initialize an array to keep track of where we have to go at each tree level.
	//The max depth expected is MAX_DEPTH.
	int stack[max_stack_size];
	for(int i = 0; i < max_stack_size; ++i)
		stack[i] = -1;

	int node_id = root_id;
	int depth = 0, max_depth = 1;
	bool node_visited[node_count] = {false};

	//a for loop instead of a while to avoid infinite loops. Since we don't expect to do more turn than node_count
	//*i* count the number of node deleted.
	int i = 0;
	while(i < node_count && node_id >= 0){
		Node const& node = nodes()[node_id];
		if (node.is_leaf()){
			i += 1;

			if(node_visited[node_id] != false){
				cout << __FILE__ << ":" << __LINE__ << " CoarseMondrianForest::tree_checker: leaf " << node_id << " already visited for the first time." << endl;
				return false;
			}
			if(node.parent >= 0){
				Node const& parent = nodes()[node.parent];
				if(parent.child_left != node_id && parent.child_right != node_id){
					cout << __FILE__ << ":" << __LINE__ << " CoarseMondrianForest::tree_checker: leaf " << node_id << " isn't the child of parent " << node.parent << endl;
					return false;
				}
			}

			node_id = node.parent;
			depth -= 1;
		}
		else{ //Internal Node
			if (stack[depth] == -1){ //going right
				stack[depth] = 0;
				depth += 1;

				if(node_visited[node_id] != false){
					cout << __FILE__ << ":" << __LINE__ << " CoarseMondrianForest::tree_checker: node " << node_id << " already visited for the first time." << endl;
					return false;
				}
				if(node.parent >= 0){
					Node const& parent = nodes()[node.parent];
					if(parent.child_left != node_id && parent.child_right != node_id){
						cout << __FILE__ << ":" << __LINE__ << " CoarseMondrianForest::tree_checker: node " << node_id << " isn't the child of parent " << node.parent << endl;
						return false;
					}
				}

				if(nodes()[node.child_right].parent != node_id){
					cout << __FILE__ << ":" << __LINE__ << " CoarseMondrianForest::tree_checker: node " << node_id << " isn't parent of " << node.child_right << endl;
					return false;
				}
				if(nodes()[node.child_left].parent != node_id){
					cout << __FILE__ << ":" << __LINE__ << " CoarseMondrianForest::tree_checker: node " << node_id << " isn't parent of " << node.child_left << endl;
					return false;
				}

				node_visited[node_id] = true;
				node_id = node.child_right;
			}
			else if (stack[depth] == 0){ //Go left
				stack[depth] = 1;
				depth += 1;
				node_id = node.child_left;
			}
			else if (stack[depth] == 1){ //Go up
				i += 1;
				stack[depth] = -1; //Reset to -1
				depth -= 1;
				node_id = node.parent;
			}
			else{
				cout << __FILE__ << ":" << __LINE__ << " CoarseMondrianForest::tree_checker: stack[" << depth << "] == " << stack[depth] << " (should be -1, 0, or 1)" << endl;
				return false;
			}
		}
	}
	return true;
}
#endif
public:
/**
 * Constructor.
 * @param lifetime The lambda parameter described in the Mondrian paper. It is used to limit the growth of the tree.
 * @param base_measure A parameter that set the initial posterior means of the parent of the root.
 * @param discount_factor The discount factor :).
 */
CoarseMondrianForest(double const lifetime, double const base_measure, double const discount_factor, int const tree_count,
		int const tm = COBBLE_MANAGEMENT,
		int const st = NODE_SIZE,
		int const sl = -1,
		int const dd = 0,
		bool const pn = false,
		int const fed = 0,
		int const fes = 0,
		double const tf = 1.0,
		bool const fdt = true,
		bool const ro = true,
		double const fep = 1.0,
		double const fc = 1.0,
		int const sh = 0,
		int const et = 0,
		int const tt = 0,
		double const mts = 1.0): tree_management(tm), size_type(st), size_limit(sl), dont_delete(dd), print_nodes(pn), fe_distribution(fed), fe_split_trigger(fes), tau_factor(tf), generate_full_point(fdt), reset_once(ro), fe_parameter(fep), fading_count(fc), split_helper(sh), extend_type(et), trim_type(tt), maximum_trim_size(mts) {
#ifdef DEBUG
	assert(tree_count >= 1 && "Must have one tree at least");
#endif
	this->lifetime = lifetime;
	this->base_measure = base_measure;
	this->discount_factor = discount_factor;
	this->tree_count = tree_count;

	int const statistics_memory = tree_count * sizeof(TreeBase);
	int const remaining_memory = max_size - statistics_memory;
	this->node_count = (remaining_memory - (remaining_memory%sizeof(Node))) / sizeof(Node);
	this->node_available = node_count;
	this->maximum_tree_count = (node_count - node_count%7)/7;
	//Init all roots as empty
	TreeBase* bases = tree_bases();
	for(int i = 0; i < tree_count; ++i)
		if(tree_management == ROBUR_MANAGEMENT){
			int limit_to_use;
			if(size_limit > 0)
				limit_to_use = size_limit;
			else
				limit_to_use = Utils::div_int<int>(node_count, tree_count);

			//The limit must be an odd number, so we adjust if needed.
			//Because node are added two by two so if the number isn't odd, we may get 1 node above the limit.
			//The number of node in a tree is always odd because there is the root which is always added alone.
			if((limit_to_use%2) == 0)
				limit_to_use -= 1;
			tree_bases()[i].reset(limit_to_use);
			size_limit = limit_to_use;
		}
		else if(tree_management == COBBLE_MANAGEMENT || tree_management == OPTIMISTIC_COBBLE_MANAGEMENT)
			tree_bases()[i].reset(size_limit);
		else
			tree_bases()[i].reset(node_count);
	for(int i = 0; i < node_count; ++i)
		nodes()[i].reset();
#ifdef DEBUG
	cout << "Tree Count = " << this->tree_count << endl;
	cout << "Node Count = " << this->node_count << endl;
	cout << "Sizeof Node = " << sizeof(Node) << endl;
	cout << "Sizeof TreeBase = " << sizeof(TreeBase) << endl;

	cout << "Node Memory = " << remaining_memory << endl;
	cout << "Tree Base Memory = " << statistics_memory << endl;
	assert(node_count >= tree_count && "Not enough memory to have one node per tree");
#endif
}

/**
 * Train all trees of the forest with a new data point.
 * Return false if a tree has failed to be trained.
 * @param features The features of the data point of size *feature_count*.
 * @param label The label of the data point.
 */
bool train(feature_type const* features, int const label){
	bool fully_trained = true;

	TreeBase* base = tree_bases();

	if(print_nodes){
		scory(features, label);
	}

	static bool checked = false;
	if(has_been_full > 1 && !checked){
		//cout << "Full memory!" << endl;
		checked = true;
	}
	//Fade the count of the average point
	for(int i = 0; i < feature_count; ++i)
		sum_features[i] = sum_features[i]*fading_count + features[i];
	count_points = count_points*fading_count + 1;

	int tree_order[tree_count];
	for(int i = 0; i < tree_count; ++i)
		tree_order[i] = i;

	if(trim_type == TRIM_FADING)
		tree_fade_counts();
	if(trim_type != TRIM_NONE && node_available <= 1 && total_count%100 == 0){
		tree_trim();
	}
	for(int i = 0; i < tree_count; ++i){
		bool has_trained = train_tree(features, label, tree_order[i]);
		if(!has_trained)
			fully_trained = false;
        #ifdef UNBOUND_OPTIMIZE
		update_posterior_count(tree_order[i], features, label);
        #endif
	}
	if(node_available <= 1)
		has_been_full += 1;

	//otter(features, label);
    #ifdef DEBUG
	if(!fully_trained)
		cout << "[WARNING] Not fully trained. (" << __FILE__ << ":" << __LINE__ << ")" << endl;
    #endif
	total_count += 1;
	TreeBase* bases = tree_bases();
	bool all_tree_grown = true;
	//for(int i = 0; i < tree_count; ++i){
		//int node_count = 0;
		//int depth = tree_depth(i, &node_count);
		//if(!bases[i].is_grown(tree_management))
			//all_tree_grown = false;
		//#ifdef DEBUG
		//double score_debug;
		//if(bases[i].statistics.ratio()){
			//score_debug = bases[i].statistics.score(false, bases[i].size);
		//}
		//else {
			//const std::type_info& ti1 = typeid(Statistic);
			//const std::type_info& ti2 = typeid(ReservoirSamplingMetrics);
			//if(ti1.hash_code() == ti2.hash_code()) //If RS
				//score_debug = bases[i].statistics.score(false, tree_count-1);
			//else
				//score_debug = bases[i].statistics.score(false);
		//}
		//#endif
		//if(size_type == DEPTH_SIZE){
			//bases[i].size = depth;
		//}
		//else if(size_type == NODE_SIZE){
			//bases[i].size = node_count;
		//}
	//}
    #ifdef DEBUG
	cout << "Nodes remaining:" << total_count << "," << node_available << endl;
	cout << "Tree count:" << tree_count << endl;
	cout << "Tree Management:" << tree_management << endl;
	#endif
	if(print_nodes && ((total_count > 0 && total_count <= 500) || (total_count > 0 && total_count%5 == 0))){
		printy();
	}
	if(node_available <= 1){ //Remove trees
		if(false && total_count > 0 && total_count%500 == 0 && tree_count < 5){
			int const index_tree_base = max_size - tree_count * sizeof(TreeBase);
			int const new_index_tree_base = index_tree_base - sizeof(TreeBase);
			int const index_last_node = node_count * sizeof(Node);
			int const space_to_free = index_last_node - new_index_tree_base;
			int const number_of_node_to_move = space_to_free < 0 ? 0 : (space_to_free - space_to_free%sizeof(Node)) / sizeof(Node) + ((space_to_free%sizeof(Node)) > 0);

			int virtual_node_count = node_count - number_of_node_to_move;
			int virtual_node_per_tree = (virtual_node_count - (virtual_node_count%(tree_count+1)))/(tree_count+1);
			int virtual_remaining = (virtual_node_count%(tree_count+1));
			for(int i = 0; i < tree_count; ++i){
				int count;
				int depth_i = tree_depth(i, &count);
				while(count > virtual_node_per_tree){
					tree_chop(i, depth_i-1);
					depth_i = tree_depth(i, &count);
				}
			}
			if(node_available > 2){
				tree_add();
				tree_bases()[tree_count-1].node_count_limit = node_available;
			}
			for(int i = 0; i < tree_count; ++i){
				tree_bases()[i].node_count_limit = virtual_node_per_tree + (i < virtual_remaining);
			}
		}
		if(false && dont_delete == DO_DELETE && total_count%500 == 0 && total_count > 0 && tree_count > 30){
			int idx_smallest_contribution = 0;
			int highest_ds = 0;
			for(int i = 0; i < tree_count; ++i){
				double const contribution = bases[i].sum_contribution/bases[i].count_contribution;
				double const best_contrib = bases[idx_smallest_contribution].sum_contribution/bases[idx_smallest_contribution].count_contribution;

				if(contribution < best_contrib)
					idx_smallest_contribution = i;
			}
			if(tree_count > 2){
				tree_delete(idx_smallest_contribution);
			}
			int node_per_trees = (node_count - (node_count%tree_count))/tree_count;
			int remaining = (node_count%tree_count);
			for(int i = 0; i < tree_count; ++i)
				bases[i].node_count_limit = node_per_trees + (i < remaining);
		}
	}
    #ifdef DEBUG
	for(int i = 0; i < tree_count; ++i){
		if(!tree_checker(i)){
			cout << "Error in CoarseMondrianForest::tree_checker for tree " << i << "." << endl;
			exit(-1);
		}
	}
	#endif
	return fully_trained;
}

void otter(feature_type const* features, int const label){
	double scores[tree_count][label_count];
	double scores_pre[tree_count][label_count];
	double full_forest_score[label_count];
	double full_depth_score[label_count];
	double full_score[label_count];
	int depths[tree_count];
	//Update internal count
	update_posterior_count();

	//Set everything to 0
	for(int i = 0; i < tree_count; ++i)
		for(int j = 0; j < label_count; ++j)
			scores[i][j] = scores_pre[i][j] = 0;
	for(int j = 0; j < label_count; ++j)
		full_forest_score[j] = full_depth_score[j] = full_score[j] = 0;

	//Compute depth
	for(int i = 0; i < tree_count; ++i)
		depths[i] = tree_depth(i, nullptr);

	for(int i = 0; i < tree_count; ++i){
		predict_tree(features, i, scores[i]);
		predict_tree(features, i, scores_pre[i], depths[i]-1);
		for(int j = 0; j < label_count; ++j){
			full_forest_score[j] += scores_pre[i][j];
			full_score[j] += scores[i][j];
		}
	}

	for(int j = 0; j < label_count; ++j)
		full_depth_score[j] = full_score[j];
	for(int j = 0; j < label_count; ++j)
		full_score[j] /= static_cast<double>(tree_count);

	double const full_forest_eval = eval_scores(full_forest_score, label);
	double full_depth_eval = 0;
	double const full_eval = eval_scores(full_score, label);

	//Brute force of the best subset of size tree_count-1
	double sum_score = 0;
	for(int i = 0; i < tree_count; ++i){
		double tmp_score[label_count];
		for(int j = 0; j < label_count; ++j)
			tmp_score[j] = (full_depth_score[j] - scores[i][j]) / static_cast<double>(tree_count-1);

		full_depth_eval += eval_scores(tmp_score, label);
	}
	full_depth_eval /= static_cast<double>(tree_count);

	count_direction += 1;
	if(full_forest_eval > full_depth_eval)
		sum_direction += 1;
	else
		sum_direction -= 1;


	#ifdef DEBUG
	//for(int j = 0; j < label_count; ++j){
		//for(int i = 0; i < tree_count; ++i)
			//cout << "Otter:" << total_count << "," << i << "," << j << "," << scores[i][j] << "," << scores_pre[i][j] << ",0" << endl;
		//cout << "Otter:" << total_count << ",-1," << j << "," << full_score[j] << "," << full_forest_score[j] << "," << full_depth_score[j] << endl;
	//}
	//for(int i = 0; i < tree_count; ++i)
		//cout << "Canard:" << total_count << "," << i << "," << Utils::index_max(scores[i], label_count) << "," << label << endl;
	//cout << "Canard:" << total_count << ",-1," << Utils::index_max(full_score, label_count) << "," << label << endl;
	//cout << "Canard:" << total_count << ",-2," << Utils::index_max(full_depth_score, label_count) << "," << label << endl;
	//cout << "Canard:" << total_count << ",-3," << Utils::index_max(full_forest_score, label_count) << "," << label << endl;
    #endif
	//Compute Contribution of each tree
	double S[tree_count];
	TreeBase* bases = tree_bases();
	double const fading_factor = 0.95;
	#ifdef DEBUG
	if(total_count > 280){
		cout << "Current_total:" << total_count << "," << full_eval << "," << full_forest_eval << "," << full_depth_eval << endl;
		cout << "Current_label:" << total_count << "," << label << endl;
		cout << "Current_score:" << total_count;
		for(int i = 0; i < label_count; ++i)
			cout << "," << full_score[i];
		cout << endl;
	}
    #endif
	for(int i = 0; i < tree_count; ++i){
		double tmp_score[label_count];

		for(int j = 0; j < label_count; ++j)
			tmp_score[j] = full_score[j] - (scores[i][j] / static_cast<double>(tree_count));

		double const parrot = eval_scores(tmp_score, label);
		S[i] = parrot - full_eval;

		//mettre à jours average S[i]
		bases[i].count_contribution = bases[i].count_contribution * fading_factor + 1;
		bases[i].sum_contribution = bases[i].sum_contribution * fading_factor + S[i];
		double canard = bases[i].sum_contribution / bases[i].count_contribution;
		#ifdef DEBUG
		if(total_count > 280)
			cout << "Current:" << total_count << "," << i << "," << canard << endl;
		#endif
	}
}
int tree_printy(int const tree_id) const{
	TreeBase const& base = tree_bases()[tree_id];
	int const root_id = base.root;

	bool ret = tree_printy(tree_id, root_id);
	return ret;
}
void node_csv(int const tree_id, int const node_id, int const depth, int const leaf, Node const& node) const{
	cout << "Narval:" << total_count << "," << tree_id << "," << node_id << "," << depth << ","
		<< node.parent << "," << node.child_right << "," << node.child_left << ","
		<< node.tau << "," << node.split_dimension << "," << node.split_value << "," << leaf << "," << node.forced_extend << ",";
	for(int i = 0; i < feature_count; ++i){
		cout << node.bound_lower[i] << "," << node.bound_upper[i] << ",";
	}
	for(int i = 0; i < label_count; ++i){
		cout << node.counters[i] << ",";
	}
	cout << endl;
}
template<int max_stack_size=100>
void tree_printy(int const tree_id, int const start_id) const{
	//Initialize an array to keep track of where we have to go at each tree level.
	//The max depth expected is MAX_DEPTH.
	int stack[max_stack_size];
	for(int i = 0; i < max_stack_size; ++i)
		stack[i] = -1;

	int node_id = start_id;
	int depth = 0, max_depth = 0;

	//a for loop instead of a while to avoid infinite loops. Since we don't expect to do more turn than node_count
	//*i* count the number of node deleted.
	int i = 0;
	int depth_id = start_id;
	while(i < node_count && depth >= 0){
		Node const& node = nodes()[node_id];
		if (node.is_leaf()){
			i += 1;
			//acknowledge the depth
			if(depth > max_depth){
				max_depth = depth;
				depth_id = node_id;
			}
			node_csv(tree_id, node_id, depth, 1, node);

			node_id = node.parent;
			depth -= 1;
		}
		else{ //Internal Node
			if (stack[depth] == -1){ //going right
				stack[depth] = 0;
				node_csv(tree_id, node_id, depth, 0, node);
				depth += 1;
				node_id = node.child_right;
			}
			else if (stack[depth] == 0){ //Go left
				stack[depth] = 1;
				depth += 1;
				node_id = node.child_left;
			}
			else if (stack[depth] == 1){ //Go up
				i += 1;
				stack[depth] = -1; //Reset to -1
				depth -= 1;
				node_id = node.parent;
			}
			#ifdef DEBUG
			else{
				cout << __FILE__ << ":" << __LINE__ << " CoarseMondrianForest::node_depth: stack[" << depth << "] == " << stack[depth] << " (should be -1, 0, or 1)" << endl;
			}
			#endif
		}
	}
}
void printy(void) const{
	for(int i = 0; i < tree_count; ++i)
		tree_printy(i, tree_bases()[i].root);
}
void scory(feature_type const* features, int const label){
	double scores[label_count];
	double sum_scores[label_count];

	update_posterior_count();
	for(int i = 0; i < tree_count; ++i){
		for(int j = 0; j < label_count; ++j)
			scores[j] = 0;
		predict_tree(features, i, scores);
		int const prediction = Utils::index_max(scores, label_count);
		cout << "Loutre:" << total_count << "," << i << "," << label << "," << prediction;
		for(int j = 0; j < label_count; ++j)
			cout << "," << scores[j];
		cout << endl;
		for(int j = 0; j < label_count; ++j)
			sum_scores[j] = scores[j];
	}
	int const prediction = Utils::index_max(sum_scores, label_count);
	cout << "Loutre:" << total_count << "," << -1 << "," << label << "," << prediction;
	for(int j = 0; j < label_count; ++j)
		cout << "," << sum_scores[j];
	cout << endl;
}
double eval_scores(double const* score, int const label) const{
	int idx_max = Utils::index_max(score, label_count); //Find the predicted label
	if(idx_max == label){ //The label is the one predicted, so we get the second class
		idx_max = 0;
		for(int i = 1; i < label_count; ++i)
			if(i != label && score[i] > score[idx_max])
				idx_max = i;
	}
	return score[label] - score[idx_max];
}
/**
 * Predict the label of the data point.
 * Return the most likely label.
 * @param features The features of the data point.
 */
int predict(feature_type const* features, double* scores = nullptr, int tree_to_use = -1){
	if(tree_to_use < 0)
		tree_to_use = tree_count;
	//Update internal count
	//NOTE uncomment for bound and trim, just to be safer
	#ifndef UNBOUND_OPTIMIZE
	if(extend_type != EXTEND_GHOST)
		update_posterior_count();
	#endif
	//The posterior mean of the forest will be the average posterior means over all trees
	//We start by computing the sum
	double tree_used = 0;
	double sum_posterior_mean[label_count] = {0};
	for(int i = 0; i < tree_to_use; ++i){
		double posterior_mean[label_count];
		//Get the  posterior means of the leaf of the data point in tree *i*
		predict_tree(features, i, posterior_mean);
		tree_used += 1;
		//Update the sum of posterior means
		for(int k = 0; k < label_count; ++k)
			sum_posterior_mean[k] += posterior_mean[k];
	}
	//Then we divide by the number trees.
	for(int k = 0; k < label_count; ++k)
		sum_posterior_mean[k] /= tree_used;

	//Finally, we look for the best label
	return Utils::index_max(sum_posterior_mean, label_count);
}

Node* get_nodes(){
	return nodes();
}
TreeBase* get_bases(){
	return tree_bases();
}
};
