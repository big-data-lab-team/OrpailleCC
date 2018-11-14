
template<class feature_type, unsigned int feature_count, unsigned int max_cluster=25,unsigned int error_threshold=2, int performance_thr=50>
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
		void incorporate(feature_type const* features, double const timestamp){
		}
		unsigned int performance(double const current_timestamp) const{
			double const current_tn = triangular_number(current_timestamp);
			double const initial_tn = triangular_number(initial_timestamp);
			double const real_tn = current_timestamp - initial_timestamp;
			double const participation = timestamp_sum * (100 / real_tn);
			return participation;
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

	void split(int const cluster_index){
		//Choose the attribute with the greatest variance
		//TODO what to do when there is no more space :]
	}
	void find_nearest_clusters(feature_type const* features, int const label, int& nearest, int& nearest_with_class){
	}
	public:
	void train(feature_type const* features, int const label){
		//Find the nearest neighbor
		//Find the nearest neighbor with the same class
		int nearest_index = -1, nearest_with_class_index = -1;
		find_nearest_clusters(features, label, nearest_index, nearest_with_class_index);
		if(nearest_with_class_index < 0){
			//Insert a new cluster for this class
		}
		//Get the cluster
		cluster& nearest = clusters[nearest_index];
		cluster& nearest_with_class = clusters[nearest_with_class_index];

		//If both nearest cluster and nearest cluster with the same class are the same, update the cluster
		if(nearest_index == nearest_with_class_index){
			//increment error_count
			nearest.error_count += 1;
			nearest.add(features, timestamp);
		}
		//else add the current record into the centroid with its class
		else{
			//decrement error_count of both
			nearest.error_count -= 1;
			nearest_with_class.error_count -= 1;
			nearest_with_class.add(features, timestamp);
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
		return 0;
	}
};
