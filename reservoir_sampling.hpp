
template<class element_type, unsigned int sample_size>
class ReservoirSampling{
	element_type sample[sample_size];
	unsigned int counter = 0;
	public:
	void add(element_type e){
	}
	element_type operator[](int i){
		return sample[i];
	}
};
