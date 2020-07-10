/**
 * NaiveBayes class implements the Naive Bayes classifier.
 * Templates:
 * - feature_type: the type to use for the feature. Either double or float since we are working with numeric attribute only.
 * - label_count: How many labels/classes/categories must be expected.
 * - feature_count: the number of features for one data point.
 * - funct: a class type that contains all needed function for the Naive Bayes.
 *   	+ log function: a logarithmic function (log2, log10, ln, ...) to avoid underflow.
 *   	+ sqrt function: a square root function.
 */
template<class feature_type, int label_count, int feature_count, class func>
class NaiveBayes{
	class GaussianEstimator{
		double weight_sum = 0, variance_sum = 0, mean = 0;
		public:
		void train(double const value, double const weight){
			if (weight_sum > 0.0) {
				weight_sum += weight;
				double const previous_mean = mean;
				mean += weight * (value - previous_mean) / weight_sum; 
				variance_sum += weight * (value - previous_mean) * (value - mean);
			} 
			else {
				mean = value;
				weight_sum = weight;
				variance_sum = 0;
			}
		}
		double probability_density(double const value) const{
			if(weight_sum <= 0)
				return 0.0;
			double const variance = (weight_sum > 1.0) ? (variance_sum / (weight_sum - 1.0)) : 0.0;
			double const stdev = func::sqrt(variance);
			if(stdev > 0){
				double const diff = value - mean;
				double const NORMAL_CONSTANT = 2.50663; //square root of 2*M_PI
				return (1.0 / (NORMAL_CONSTANT * stdev)) * exp(-(diff * diff / (2.0 * stdev * stdev)));
			}
			return (value == mean) ? 1.0 : 0.0;
		}
	};
	//The sum of the value in features_size
	int sum_feature_size = 0;
	//The list of estimators for each feature and each label.
	GaussianEstimator counters[label_count * feature_count];
	double label_weights[label_count] = {0};
	double total_weights = 0;
	//The smoothing value
	double smoothing;

	public:
		/*
		* Constructor.
		* @param smoothing the smoothing value to use.
		*/
		NaiveBayes(double const smoothing=0.0){
			//Set the smoothing
			this->smoothing = smoothing;
		}
		/*
		* Train the model with one data point.
		* @param features an array of size *feature_count* that contain how many value there is for each feature.
		* @param smoothing the smoothing value to use.
		* @param weight the weight to give to this data point (default: 1.0).
		*/
		bool train(feature_type const* features, int const label, double const weight = 1.0){
			for(int i = 0; i < feature_count; ++i)
				counters[label * feature_count + i].train(features[i], weight);
			label_weights[label] += weight;
			total_weights += weight;
			return true;
		}
		/*
		* Predict the label of the given data point.
		* @param features an array of size *feature_count* that contains the value of the data point.
		* @param scores a pointer of size *label_count* that will store the score for each label.
		*/
		int predict(feature_type* features, double* scores = nullptr){
			double local_score[label_count] = {0};	
			//Accumulate the probability for each label
			for(int l = 0; l < label_count; ++l){
				local_score[l] = func::log(label_weights[l] / total_weights);
				for(int f = 0; f < feature_count; ++f)//Add the log to avoid overflow
					local_score[l] += func::log(counters[l * feature_count + f].probability_density(features[f]));
			}

			int max_l = 0;
			for(int l = 1; l < label_count; ++l){
				if(local_score[max_l] < local_score[l])
					max_l = l;
			}
			if(scores != nullptr){
				for(int l = 0; l < label_count; ++l)
					scores[l] = local_score[l];
			}
			return max_l;
		}
		/*
		* Change the value of the smoothing.
		* @param val the new value.
		*/
		void set_smoothing(double const val){
			smoothing = val;
		}
		/*
		* Get the smoothing value.
		* @return the current value for the smoothing.
		*/
		double get_smoothing(void){
			return smoothing;
		}
};
