#include <iostream>
using namespace std;
template<class feature_type, unsigned int feature_count, unsigned int max_cluster=25,unsigned int error_threshold=2, int performance_thr=50, int empty_class=-1>
class MCNN{
	struct cluster{
		feature_type features_square_sum[feature_count];
		feature_type features_sum[feature_count];
		double timestamp_square_sum;
		double timestamp_sum;
		unsigned int data_count;
		int label;
		int error_count;
		double initial_timestamp;
		double sqrt_local(double const val) const{
			return 0.0;
		}
		double variance(unsigned int const features_idx) const{
			double const a = features_square_sum[features_idx] / data_count;
			double const b = features_sum[features_idx] / data_count;
			double const ret=sqrt_local(a - b*b);
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
			error_count = error_threshold + 1;
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
	cluster clusters[max_cluster];
	bool active[max_cluster] = {false};
	int count_active_cluster = 0;
	double timestamp = 0;

	template<class type>
	double triangular_number(type t) const{
		return ((t*t + t) * 0.5);
	}

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

		//Choose the attribute with the greatest variance
		double highest_variance = 0;
		unsigned int best_idx_variance = -1;
		for(int i = 0; i < feature_count; ++i){
			double const variance = clusters[cluster_idx].variance(i);
			if(variance > highest_variance){
				highest_variance = variance;
				best_idx_variance = i;
			}
		}


		clusters[new_idx]=clusters[cluster_idx];
		feature_type centroid[feature_count];
		clusters[cluster_idx].centroid(centroid);

		//clusters[new_idx].initial_timestamp = clusters[cluster_idx].initial_timestamp = 0;
		//clusters[new_idx].timestamp_sum = clusters[cluster_idx].timestamp_sum = 0;
		//clusters[new_idx].timestamp_square_sum = clusters[cluster_idx].timestamp_square_sum = 0;
		//
		//
		//clusters[new_idx].features_sum


	}
	double euclidean_distance(feature_type const* e1, feature_type const* e2) const{
		double squared_sum = 0;
		for(int i = 0; i < feature_count; ++i)
			squared_sum += (e1[i] - e2[i]) * (e1[i] - e2[i]);
		//NOTE: Not really a distance :D
		return squared_sum;
	}
	void find_nearest_clusters(feature_type const* features, int const label, int& nearest, int& nearest_with_class){
		find_nearest_clusters(features, nearest);
		if(nearest >= 0 && clusters[nearest].label == label){
			nearest_with_class = nearest;
			return;
		}

		feature_type centroid[feature_count];
		int nearest_cluster = -1;
		double shortest_distance = 1000000;
		for(int cluster_idx = 0; cluster_idx < max_cluster; ++cluster_idx){
			if(!active[cluster_idx] || clusters[cluster_idx].label != label)
				continue;
			clusters[cluster_idx].centroid(centroid);
			double const distance = euclidean_distance(features, centroid);
			if(distance < shortest_distance){
				nearest_cluster = cluster_idx;
				shortest_distance = distance;
			}
		}
		nearest_with_class = nearest_cluster;
	}
	void find_nearest_clusters(feature_type const* features, int& nearest){
		feature_type centroid[feature_count];
		int nearest_cluster = -1;
		double shortest_distance = 1000000;
		for(int cluster_idx = 0; cluster_idx < max_cluster; ++cluster_idx){
			if(!active[cluster_idx])
				continue;
			clusters[cluster_idx].centroid(centroid);
			double const distance = euclidean_distance(features, centroid);
			if(distance < shortest_distance){
				nearest_cluster = cluster_idx;
				shortest_distance = distance;
			}
		}
		nearest = nearest_cluster;
	}
	public:
	void train(feature_type const* features, int const label){
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
					clusters[i].initialize(features, label, timestamp);
					return;
				}
			}
			//If we are here, there was already `max_cluster` clusters active.
			//TODO don't know what to do
			printf("There is not enough cluster for all classes.\n");
			assert(false);
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
			if(nearest_with_class.error_count < error_threshold){
				split(nearest_with_class_index);
			}
			if(nearest.error_count < error_threshold){
				split(nearest_index);
			}
		}
	}
	int predict(feature_type const* features){
		assert(features != NULL);
		int nearest_index = -1;
		find_nearest_clusters(features, nearest_index);
		if(nearest_index < 0)
			return empty_class;
		assert(nearest_index < max_cluster);
		return clusters[nearest_index].label;
	}
};
