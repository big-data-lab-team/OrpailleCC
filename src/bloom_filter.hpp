#include <cassert>

//Typedef to simplify the use of the hash function given by the user.
#define BYTE_SIZE (sizeof(unsigned char))

/**
 * The BloomFilter class implement the Bloom filter algorithm.
 * - element_type: the type of element to take as input.
 * - bit_size: the size of the Bloom filter in bit.
 * - hash_count: the number of hash function to use.
 */
template<class element_type, int bit_size, int hash_count>
class BloomFilter{
	typedef unsigned int (*hash_function_t)(element_type*);
	unsigned char bits[bit_size];
	hash_function_t hashs[hash_count];
	unsigned int const real_size = ((bit_size - (bit_size%BYTE_SIZE)) / BYTE_SIZE) + ((bit_size%BYTE_SIZE) > 0);

	public:
	/**
	 * Constructor of the BloomFilter.
	 * @param h an array of size hash_count with the list of hash function to use.
	 */
	BloomFilter(hash_function_t* h){
		assert(h != nullptr);
		assert(bit_size >= 1);
		assert(hash_count >= 1);
		for(int i = 0; i < hash_count; ++i){
			hashs[i] = h[i];
			assert(hashs[i] != nullptr);
		}
		clear();
	}
	/*
	 * Add an element to the Bloom Filter
	 * @param bf The BloomFilter object.
	 * @param element The pointer to the new element to add
	 */
	void add(element_type* element){
		assert(element != nullptr);
		for(int i = 0; i < hash_count; ++i)
			set_bit_to_one(hashs[i](element));
	}
	/*
	 * Add an element to the Bloom Filter
	 * @param bf The BloomFilter object.
	 * @param element The new element to add
	 */
	void add(element_type element){
		add(&element);
	}

	/*
	 * Lookup if an element is in the BloomFilter.
	 * @param element The pointer to the element to check.
	 * @return true if the element is possibly in the filter.
	 */
	bool lookup(element_type* element){
		assert(element != nullptr);
		for(int i = 0; i < hash_count; ++i)
			if(get_bit(hashs[i](element)) == 0)
				return false;
		return true;
	}
	/*
	 * Lookup if an element is in the BloomFilter.
	 * @param element The element to check.
	 * @return true if the element is possibly in the filter.
	 */
	bool lookup(element_type element){
		return lookup(&element);
	}

	/*
	 * Empty the BloomFilter object by setting all bits to zero.
	 * @param bf The BloomFilter object.
	 */
	void clear(void){
		for(int i = 0; i < real_size; ++i)
			bits[i] = 0;
	}

	/*
	 * Set the filter with the bit array given in parameter
	 * @param new_bits the new array of bit.
	 */
	void set(unsigned char* new_bits){
		assert(new_bits != nullptr);
		for(int i = 0; i < real_size; ++i)
			bits[i] = new_bits[i];
	}

	private:
	/*
	 * Set a specific bit to 1 in the bit array.
	 * @param index the index of the bit to set to one.
	 */
	void set_bit_to_one(int const index){
		assert(index >= 0 && index < bit_size);
		unsigned int const mod = index % BYTE_SIZE;
		unsigned int const real_index = (index - mod) / BYTE_SIZE;
		bits[real_index] = bits[real_index] | (1 << mod);
	}
	/*
	 * Access the value of a bit in the bit array.
	 * @param index the index of the bit to access.
	 */
	int get_bit(int const index) const{
		assert(index >= 0 && index < bit_size);
		unsigned int const mod = index % BYTE_SIZE;
		unsigned int const real_index = (index - mod) / BYTE_SIZE;
		return ((bits[real_index] & (1 << mod)) != 0);
	}
};
