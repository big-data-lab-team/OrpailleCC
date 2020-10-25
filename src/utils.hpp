class Utils{
	public:
	/**
	 * Uniformly pick a random index following the probability distribution given by *probabilities*.
	 * Example: probabilities = [0.2, 0.5, 1.0], then index 0 has 20% chance of being picked, index 1 has 30% chance and index 2 has 50%.
	 * Templates:
	 * - func: a class type that contains all underlying this one.
	 *   	+ rand_uniform function: A function that pick uniformly a random number between [0,1[.
	 * @param probabilities An array that contains the distribution of probabilities. The final value of the index should be equal to 1.
	 * @param size The size of the array *probabilities*.
	 */
	template<class func>
	static int pick_from_distribution(double const*const probabilities, const int size){
		double const u = func::rand_uniform(); //Sample a random number between [0,1[
		//NOTE improve with a log search :]
		for(int i = 0; i < size; ++i){
			if(u <= probabilities[i])
				return i;
		}
		return size;//NOTE: this should never happen unless the last element of *probabilities* is less than 1
	}
	/**
	 * Turn an array of values into a sum of probabilities according to these values.
	 * [6, 15, 9] -> [0.2, 0.5, 0.3] -> [0.2, 0.7, 1.0]
	 * @param values An array of values. This array will store the result.
	 * @param size The size of *values*.
	 * @param sum The sum of the array. If sum equals -1, the sum is computed.
	 */
	static void turn_array_into_probability(double* values, int const size, int sum = -1) {
		//Compute the sum unless sum is already computed
		if(sum == -1){
			sum = 0;
			for(int i = 0; i < size; ++i)
				sum += values[i];
		}
		//Set the first element of the array to its probability
		values[0] = (values[0]/sum);

		//Itirate to create the sum of probabilities
		for(int i = 1; i < size; ++i)
			values[i] = values[i-1] + (values[i]/sum);
	}
	/**
	 * A function that pick a random number according to an exponential law.
	 * Take one double parameter, which is lambda. See https://en.wikipedia.org/wiki/Exponential_distribution for mor information about exponential distribution.
	 * Templates:
	 * - func: a class type that contains all underlying this one.
	 *   	+ rand_uniform function: A function that pick uniformly a random number between [0,1[.
	 *   	+ log function: A function that run the natural logarithm.
	 * @param rate The lambda parameter
	 */
	template<class func>
	static double rand_exponential(double const rate){
		double const u = func::rand_uniform(); //Sample a random number between [0,1[
		return func::log(1-u) / (-rate); //turn it into the exponential distribution
	}
	//An inline function that compute the minimum between two numbers
	template<class T>
	inline static bool min(T const a, T const b){
		return (a < b ? a : b);
	}
	template<class T>
	inline static bool max(T const a, T const b){
		return (a > b ? a : b);
	}
	template<class func>
	inline static double expm1(double const a){
		return func::exp(a) - 1;
	}
};
