#include <cmath>

/**
 * A class that implement the reservoir sampling algorithm.
 *
 * The templates element_type indicate what type of element the class should sample, sample_size inidcate the size of the sample and random_function
 * give the class a random function that should uniformly return values between 0 and 1.
 */
template<class element_type, unsigned int sample_size, double (*random_function)(), class funct>
class ChainedReservoirSampling{
	struct node {
		node* next;
		element_type element;
		unsigned int timestamp;
	};
	node sample[sample_size];
	unsigned int next[sample_size] = {0};
	unsigned int counter = 0;

	/**
	 * Round d to the closest integer
	 * @param d the double to round
	 */
	double round(double const d) {
		  return floor(d + 0.5);
	}
	void clear_chain(node* start){
		if(start == NULL)
			return;
		start->timestamp = 0;
		//TODO improve this recursive code lazy boy -_-"
		clear_chain(start->next);
	}
	public:
	/**
	 * Sample one new element into the sample. This new element may not be added.
	 * @param e The new element to eventualy add to the sample.
	 */
	void add(element_type e, unsigned int const timestamp){
		if(counter < sample_size){
			sample[counter].element = e;
			sample[counter].timestamp = timestamp;
			sample[counter].next = NULL;
			next[counter] = counter + 1 + round(random_function() * (sample_size-1));
		}
		else{
			double const threshold = (double)sample_size / (double)counter;
			double const rnd = random_function();
			if(rnd < threshold){
				int const index = round(random_function() * (sample_size-1));
				sample[index].element = e;
				sample[index].timestamp = timestamp;
				clear_chain(sample[index].next); //NOTE we do not free the memory here, but it might be interesting to think about it
				next[index] = counter + 1 + round(random_function() * (sample_size-1));
			}
		}
		//TODO check next ?
		counter += 1;
	}
	/**
	 * A const operator to access the sample like an array.
	 * @param i the index to access.
	 */
	element_type const& operator[](int const i) const{
		return sample[i].element;
	}
	/**
	 * Operator to access the sample like an array.
	 * @param i the index to access.
	 */
	element_type& operator[](int const i) {
		return sample[i].element;
	}
};

