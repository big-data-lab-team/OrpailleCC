/**
 * Implement a Reservoir class which mostly act like an array.
 * - element_type: the type of element to store.
 * - sample_size: the size of the reservoir.
 */
template<class element_type, unsigned int sample_size>
class Reservoir{
	protected:
	element_type sample[sample_size];

	public:
	/**
	 * A const operator to access the sample like an array.
	 * @param i the index to access.
	 */
	element_type const& operator[](int const i) const{
		return sample[i];
	}
	/**
	 * Operator to access the sample like an array.
	 * @param i the index to access.
	 */
	element_type& operator[](int const i) {
		return sample[i];
	}
};

/**
 * Implement the Reservoir Sampling algorithm.
 * - element_type: the type of element to store.
 * - sample_size: the size of the reservoir.
 * - func: a class type that contains all needed function for the Reservoir Sampling.
 *   	+ random function: A function that returns a random number between 0 and 1.
 *   	+ floor function: A function that floor a floating point number.
 */
template<class element_type, unsigned int sample_size, class func>
class ReservoirSampling : public Reservoir<element_type, sample_size>{
	//Count the number of element seen so far
	unsigned int counter = 0;

	public:
	/**
	 * Sample one new element into the sample. This new element may not be added.
	 * Return the index of the new element. -1 otherwise.
	 * @param e The new element to eventualy add to the sample.
	 */
	inline int add(element_type const& e){
		int const idx = sample_index();
		if(idx >= 0){
			this->sample[idx] = e;
		}
		return idx;
	}
	/**
	 * Sample one new element into the sample. This new element will have to be added by the user.
	 * Return the index of the new element. -1 otherwise.
	 */
	int sample_index(void){
		int idx = -1;
		//If the sample is not full yet, just use the last index.
		if(counter < sample_size){
			idx = counter;
		}
		else{
			//We use counter+1 because the current item is not counted in counter yet
			double const threshold = static_cast<double>(sample_size) / static_cast<double>(counter+1);
			double const rnd = func::random();
			if(rnd < threshold)
				idx = func::floor(func::random() * static_cast<double>(sample_size));
		}
		counter += 1;
		return idx;
	}
	/**
	 * Return the number of item already in the Reservoir.
	 */
	inline unsigned int count(void) const{
		return counter;
	}
};

/**
 * Implement the Reservoir Sampling algorithm with a biased exponential function.
 * - element_type: the type of element to store.
 * - sample_size: the size of the reservoir.
 * - func: a class type that contains all needed function for the Reservoir Sampling.
 *   	+ random function: A function that returns a random number between 0 and 1.
 *   	+ floor function: A function that round a floating point number.
 */
template<class element_type, unsigned int sample_size, class func>
class ExponentialReservoirSampling : public Reservoir<element_type, sample_size>{
	//Count the number of element seen so far
	unsigned int counter = 0;

	public:
	/**
	 * Sample one new element into the sample.
	 * Return the index of the new element. 
	 * @param e The new element to add to the sample.
	 */
	inline int add(element_type e){
		int const idx = sample_index();
		if(idx >= 0){
			this->sample[idx] = e;
		}
		return idx;
	}
	/**
	 * Sample one new element into the sample. This new element will have to be added by the user.
	 * Return the index of the new element. -1 otherwise.
	 */
	int sample_index(void){
		double const filling_ratio = static_cast<double>(counter)/static_cast<double>(sample_size);
		double const remove_element = func::random();
		int index;
		if(remove_element < filling_ratio){
			double const rdn = func::random();
			//NOTE: index range from [0, counter[.
			index = func::floor(rdn * static_cast<double>(counter));
		}
		else{
			index = counter;
			counter += 1;
		}
		return index;
	}
	/**
	 * Return the number of item already in the Reservoir.
	 */
	inline unsigned int count(void) const{
		return counter;
	}
};
