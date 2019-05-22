#include <cassert>
#include <cmath>
#include <iostream>
using namespace std;
//https://github.com/mahmoodshakir/Micro-Cluster-Nearest-Neighbour-MC-NN-Algorithm
/*
 * Implement the MC-NN algorithm.
 * - feature_type: the type of the features. They must have all the same type.
 * - feature_count: The number of feature per data point.
 * - max_cluster: The maximum number of cluster to use (default: 25).
 * - error_threshold: The number of misclassification before the cluster is split.
 * - performance_thr: the threshold to remove old cluster. (not used yet).
 * - empty_class: the value of an empty class.
 */
template<class feature_type, unsigned int feature_count, unsigned int max_cluster=25,unsigned int error_threshold=2, int performance_thr=50, int empty_class=-1>
class MCNN{
	//Define an internal definition of a micro-cluster
	struct cluster{
		feature_type features_square_sum[feature_count];//CF2X
		feature_type features_sum[feature_count];//CF1X
		double timestamp_square_sum; //CF2T
		double timestamp_sum; //CF1T
		unsigned int data_count;
		int label;
		int error_count;
		double initial_timestamp;
		double variance(unsigned int const features_idx) const{
			double const a = (double)features_square_sum[features_idx] / (double)data_count; //CF2X
			double const b = (double)features_sum[features_idx] / (double)data_count; //CF1X
			double const ret=a - b*b;
			return ret;
		}
		void incorporate(feature_type const* features, double const timestamp){
			timestamp_sum += timestamp;
			timestamp_square_sum += timestamp * timestamp;
			data_count += 1;
			for(int i = 0; i < feature_count; ++i){
				features_sum[i] += features[i];
				features_square_sum[i] += features[i]*features[i];
			}
		}
		double performance(double const current_timestamp) const{
			double const current_tn = triangular_number(current_timestamp);
			double const initial_tn = triangular_number(initial_timestamp);
			double const real_tn = current_timestamp - initial_timestamp;
			double const participation = timestamp_sum * (100 / real_tn);
			return participation;
		}
		void initialize(feature_type const* features, int const label, double const timestamp){
			initial_timestamp = timestamp;
			timestamp_sum = timestamp;
			timestamp_square_sum = timestamp * timestamp;
			data_count = 1;
			error_count = error_threshold; //The error count start at the threshold (contrary to what is says in the paper)
			this->label = label;
			for(int i = 0; i < feature_count; ++i){
				features_sum[i] = features[i];
				features_square_sum[i] = features[i]*features[i];
			}
		}
		void centroid(feature_type* features) const{
			for(int i = 0; i < feature_count; ++i){
				features[i] = (double)features_sum[i] / (double)data_count;
			}
		}
		cluster& operator=(const cluster& other){
			if(this != &other){
				for(int i = 0; i < feature_count; ++i){
					features_square_sum[i] = other.features_square_sum[i];
					features_sum[i] = other.features_sum[i];
				}
				timestamp_square_sum = other.timestamp_square_sum;
				timestamp_sum = other.timestamp_sum;
				data_count = other.data_count;
				label = other.label;
				error_count = other.error_count;
				initial_timestamp = other.initial_timestamp;
			}
			return *this;
		}
	};

	//An array with the maximum number of micro-cluster available
	cluster clusters[max_cluster];
	//An array to check if the cluster *i* is active of empty. note: we can use false to set the array because false == 0.
	bool active[max_cluster] = {false};
	//Count the number of cluster active
	int count_active_cluster = 0;
	double timestamp = 0;

	//The triangular function
	template<class type>
	double triangular_number(type t) const{
		return ((t*t + t) * 0.5);
	}

	//Function to split a cluster
	void split(int const cluster_idx){
		int new_idx = -1;
		for(int i = 0; i < max_cluster; ++i){
			if(!active[i] && i != cluster_idx){
				new_idx = i;
				break;
			}
		}
		if(new_idx < 0){
			//TODO what to do when there is no more space :]
		}

		//Choose the attribute with the greatest variance, then do the split on it
		double highest_variance = 0;
		unsigned int split_index = -1;
		for(int i = 0; i < feature_count; ++i){
			double const variance = clusters[cluster_idx].variance(i);
			if(variance > highest_variance || split_index == -1){
				highest_variance = variance;
				split_index = i;
			}
		}
		highest_variance = sqrt_local(highest_variance);
		//Create two separate feature data points based on the centroid of the original cluster
		feature_type cluster_minus[feature_count], cluster_plus[feature_count];
		for(int i = 0; i < feature_count; ++i){
			cluster_minus[i] = cluster_plus[i] = clusters[cluster_idx].features_sum[i] / clusters[cluster_idx].data_count;
		}

		//Update the spliting attribut
		cluster_minus[split_index] -= highest_variance;
		cluster_plus[split_index] += highest_variance;
		
		//TODO: round somehow
		int old_time = (clusters[cluster_idx].timestamp_sum / clusters[cluster_idx].data_count);

		clusters[new_idx].initialize(cluster_minus, clusters[cluster_idx].label, old_time);
		clusters[cluster_idx].initialize(cluster_plus, clusters[cluster_idx].label, old_time);
		active[new_idx] = true;
		count_active_cluster += 1;
	}
	//Compute the squared distance between two data point
	double euclidean_distance(feature_type const* e1, feature_type const* e2) const{
		double squared_sum = 0;
		for(int i = 0; i < feature_count; ++i)
			squared_sum += (e1[i] - e2[i]) * (e1[i] - e2[i]);
		//NOTE: Not really a distance :D
		return squared_sum;
	}
	//Find the nearest cluster as well as the nearest cluster with the same class given a data point
	void find_nearest_clusters(feature_type const* features, int const label, int& nearest, int& nearest_with_class) const{
		//First find the nearest cluster
		double distance_nearest = -1;
		find_nearest_clusters(features, nearest, &distance_nearest);
		//If it turns out the nearest was the one with the same label, just return both
		if(nearest >= 0 && clusters[nearest].label == label){
			nearest_with_class = nearest;
			return;
		}
		//Otherwise, we have to found it.
		feature_type centroid[feature_count];
		int nearest_cluster = -1;
		double shortest_distance = 1000000;
		for(int cluster_idx = 0; cluster_idx < max_cluster; ++cluster_idx){
			//Only check the distance with the cluster with the same label.
			if(!active[cluster_idx] || clusters[cluster_idx].label != label)
				continue;
			//NOTE: finding both the nearest and the nearest with class could be done in one pass, but I choose to use 2 different functions to make it clearer.
			clusters[cluster_idx].centroid(centroid);
			double const distance = euclidean_distance(features, centroid);
			if(distance < shortest_distance){
				nearest_cluster = cluster_idx;
				shortest_distance = distance;
			}
		}
		nearest_with_class = nearest_cluster;

		//If the distance to the micro-cluster with the same class is the same as an other cluster which was picked, then replace nearest.
		if(shortest_distance == distance_nearest)
			nearest = nearest_with_class;
	}
	//Find the nearest cluster given a data point
	void find_nearest_clusters(feature_type const* features, int& nearest, double* shortest = nullptr) const{
		feature_type centroid[feature_count];
		int nearest_cluster = -1;
		double shortest_distance = 1000000;
		//Loop over the clusters
		for(int cluster_idx = 0; cluster_idx < max_cluster; ++cluster_idx){
			//If the cluster is empty, skip it
			if(!active[cluster_idx])
				continue;
			//Compute the centroid of the cluster
			clusters[cluster_idx].centroid(centroid);
			//Compute the squared euclidean distance
			double const distance = euclidean_distance(features, centroid);
			if(distance < shortest_distance){
				nearest_cluster = cluster_idx;
				shortest_distance = distance;
			}
		}
		//NOTE: we should always have a nearest cluster except for the first data point.
		nearest = nearest_cluster;
		if(shortest != nullptr)
			*shortest = shortest_distance;
	}
	//The sqrt implementation if needed.
	double sqrt_local(double const val) const{
		return sqrt(val); //TODO: to change
	}
	public:
	/*
	 * Train the model with a new data point.
	 * @param features An array of features.
	 * @param label The class for this data point.
	 * @return Return false if there is no more space for a new cluster.
	 */
	bool train(feature_type const* features, int const label){
		assert(features != NULL);
		//Find the nearest neighbor
		//Find the nearest neighbor with the same class
		int nearest_index = -1, nearest_with_class_index = -1;
		find_nearest_clusters(features, label, nearest_index, nearest_with_class_index);
		if(nearest_with_class_index < 0){
			//Insert a new cluster for this class
			for(int i = 0; i < max_cluster; ++i){
				if(active[i] == false){
					active[i] = true;
					count_active_cluster += 1;
					clusters[i].initialize(features, label, timestamp);
					return true;
				}
			}
			//If we are here, there was already `max_cluster` clusters active.
			//TODO don't know what to do :]
			return false;
		}
		//Get the cluster
		cluster& nearest = clusters[nearest_index];
		cluster& nearest_with_class = clusters[nearest_with_class_index];

		//If both nearest cluster and nearest cluster with the same class are the same, update the cluster
		if(nearest_index == nearest_with_class_index){
			//increment error_count
			nearest.error_count += 1;
			nearest.incorporate(features, timestamp);
		}
		//else add the current record into the centroid with its class
		else{
			//decrement error_count of both
			nearest.error_count -= 1;
			nearest_with_class.error_count -= 1;
			nearest_with_class.incorporate(features, timestamp);
			//If one of them reach error_threshold, the cluster is split
			if(nearest_with_class.error_count <= 0){
				split(nearest_with_class_index);
			}
			if(nearest.error_count <= 0){
				split(nearest_index);
			}
		}
		return true;
	}
	/**
	 * Predict the class of a data point.
	 * @param features The feature values of the data point.
	 * @return Return the class predicted.
	 */
	int predict(feature_type const* features) const{
		assert(features != NULL);
		int nearest_index = -1;
		find_nearest_clusters(features, nearest_index);
		if(nearest_index < 0)
			return empty_class;
		assert(nearest_index < max_cluster);
		return clusters[nearest_index].label;
	}
	/**
	 * Return the number of active cluster.
	 * @return The number of active clusters.
	 */
	int count_clusters(void) const{
		return count_active_cluster;
	}
};
