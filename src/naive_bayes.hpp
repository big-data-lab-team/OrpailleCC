/**
 * NaiveBayes class implements the Naive Bayes classifier.
 * Templates:
 * - label_count: How many labels/classes/categories must be expected.
 * - feature_count: the number of features for one data point.
 * - funct: a class type that contains all needed function for the Naive Bayes.
 *   	+ log function: a logarithmic function (log2, log10, ln, ...) to avoid underflow.
 *   	+ malloc function: a function that allocate memory.
 */
template<int label_count, int feature_count, class func>
class NaiveBayes{
	//The number of values for each feature
	int features_size[feature_count];
	//The sum of the value in features_size
	int sum_feature_size = 0;
	/*A 3D array
	 * 1D : label and has a size of sum_feature_size
	 * 2D : features
	 * 3D : the value of the feature for each feature
	 */
	int* counters;
	//Count the number of occurence per label (for prior probabilities)
	int* counters_label;
	//The smoothing value
	double smoothing;
	public:
		/*
		* Constructor.
		* @param features_size an array of size *feature_count* that contain how many value there is for each feature.
		* @param smoothing the smoothing value to use.
		*/
		NaiveBayes(int features_size[feature_count], double const smoothing=0.0){
			//Set the smoothing
			this->smoothing = smoothing;
			//Compute the sum of values of features
			for(int i = 0; i < feature_count; ++i){
				this->features_size[i] = features_size[i];
				sum_feature_size += features_size[i];
			}
			//Allocate and initialize the counters
			counters = (int*)func::malloc(label_count*sum_feature_size*sizeof(int));
			for(int i = 0; i < label_count*sum_feature_size; ++i)
				counters[i] = 0;

			//Allocate and initialize the counters of the label
			counters_label = static_cast<int*>(func::malloc(label_count*sizeof(int)));
			for(int i = 0; i < label_count; ++i)
				counters_label[i] = 0;
		}
		/*
		* Train the model with one data point.
		* @param features an array of size *feature_count* that contain how many value there is for each feature.
		* @param smoothing the smoothing value to use.
		*/
		bool train(int const* features, int const label){
			//Find the first index for the counter of this label
			int counter_idx = label * sum_feature_size;
			//For each feature, increase the counter corresponding to the value
			for(int i = 0; i < feature_count; ++i){
				if(features[i] >= features_size[i])
					return false;
				//Calculate the index of the counter
				int const tmp_idx = counter_idx + features[i];
				//Increase the counter
				counters[tmp_idx] += 1;
				//Increase to the next feature
				counter_idx += features_size[i];
			}
			//Increase the count of occurence for *label*
			counters_label[label] += 1;
			return true;
		}
		/*
		* Predict the label of the given data point.
		* @param features an array of size *feature_count* that contains the value of the data point.
		* @param scores a pointer of size *label_count* that will store the score for each label.
		*/
		int predict(int const* features, double* scores = nullptr){
			//Compute the total count of data points
			double sum_datapoints = 0;
			for(int label = 0; label < label_count; ++label)
				sum_datapoints += counters_label[label];

			//Declare the score for each label
			double label_scores[label_count];

			for(int label = 0; label < label_count; ++label){
				//Initialize the score for the label with the prior probability
				double const label_numerator = static_cast<double>(counters_label[label]) + static_cast<double>(sum_feature_size)*smoothing;
				label_scores[label] = func::log(label_numerator / sum_datapoints);
				//Set the starting index for the counters for the current label
				int const label_idx = label * sum_feature_size;
				//feature_idx stores the starting index for the counters of a specific feature (each feature has a counter for each of its values)
				int feature_idx = 0;
				for(int feature = 0; feature < feature_count; ++feature){
					//The sum of all the counter for this feature
					double sum_current_feature = static_cast<double>(features_size[feature]) * smoothing;
					//Get the value of the counter for the value of the current feature
					double const current_feature_count = counters[label_idx + feature_idx + features[feature]] + smoothing;
					//Compute the sum for the current feature
					for(int feature_value = 0; feature_value < features_size[feature]; ++feature_value){
						int const idx = label_idx + feature_idx + feature_value;
						sum_current_feature += counters[idx];
					}
					//Add the score for the current feature. Use a log function to avoid underflow
					label_scores[label] += func::log(current_feature_count / sum_current_feature);
					//Increase *feature_idx* to the index of the next feature
					feature_idx += features_size[feature];
				}
			}

			//Get the best label
			int best_score_idx = 0;
			for(int label = 1; label < label_count; ++label)
				if(label_scores[label] > label_scores[best_score_idx])
					best_score_idx = label;

			//If the label scores are needed, we copy them
			if(scores != nullptr)
				for(int label = 0; label < label_count; ++label)
					scores[label] = label_scores[label];
			return best_score_idx;
		}
		/*
		 * Destructor
		 */
		~NaiveBayes(){
			func::free(counters);
			func::free(counters_label);
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
