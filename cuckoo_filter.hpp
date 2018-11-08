#include <cassert>
#include <type_traits>
#include <cmath>

#include <iostream>
#include <utility>

#define BYTE_SIZE (sizeof(unsigned char))
template<class element_type, int bucket_count, int bucket_size, int entry_size, class funct, double (*random_function)(), int empty_value=0>
class CuckooFilter{
	//Typedef to simplify the use of the hash function given by the user.
	typedef unsigned char fingerprint_t;
	unsigned int const element_size = sizeof(element_type);
	static unsigned int const total_size = ceil((double)bucket_count*bucket_size*entry_size / (double)BYTE_SIZE);
	unsigned char filter[total_size] = {0};

	unsigned char get_bit(unsigned int const bit_index) const{
		unsigned int const mod = bit_index % BYTE_SIZE;
		unsigned int const byte_index = (bit_index - mod) / BYTE_SIZE;
		return (filter[byte_index] & (1 << mod) != 0);
	}
	void set_bit(unsigned int const bit_index, unsigned int const value){
		unsigned int const mod = bit_index % BYTE_SIZE;
		unsigned int const byte_index = (bit_index - mod) / BYTE_SIZE;
		if(value == 0)
			filter[byte_index] = filter[byte_index] & (~(1 << mod));
		else
			filter[byte_index] = filter[byte_index] | (1 << mod);
	}
	fingerprint_t get_entry(unsigned int const bucket_index, unsigned int const entry_index) const{
		assert(bucket_index >= 0 && bucket_index < bucket_count);
		assert(bucket_size >= 0 && entry_index < bucket_size);
		/*
		 * (bucket_size * entry_size) is equal to the size of one bucket in bit
		 * so (bucket_index * bucket_size * entry_size) gives the index of the first bit of bucket bucket_index
		 */
		unsigned int const bit_index = bucket_index * bucket_size * entry_size + entry_index * entry_size;
		unsigned int const mod = bit_index % BYTE_SIZE;
		unsigned int const byte_index = (bit_index - mod) / BYTE_SIZE;
		unsigned int const last_bit_index = bit_index + entry_size;
		unsigned int const last_mod = last_bit_index % BYTE_SIZE;
		unsigned int const last_byte_index = (last_bit_index - last_mod) / BYTE_SIZE;
		assert(bit_index >= 0 && bit_index < bucket_count * bucket_size * entry_size);
		//last_bit_index is the last bit to set plus one, that is why there is the <= on the next assert
		assert(last_bit_index >= 0 && last_bit_index <= bucket_count * bucket_size * entry_size);
		assert(bit_index <= last_bit_index);
		/*
		 * The first bit to set is the bit `mod` in the element `byte_index` of `filter`
		 */
		fingerprint_t tmp = 0;
		for(int count = 0, bi = bit_index; bi < last_bit_index; ++bi, ++count){
			unsigned char value = get_bit(bi);
			tmp = tmp | (value << count);	
		}
		return tmp;
	}
	void set_entry(unsigned int const bucket_index, unsigned int const entry_index, fingerprint_t const fp){
		assert(bucket_index >= 0 && bucket_index < bucket_count);
		assert(bucket_size >= 0 && entry_index < bucket_size);
		/*
		 * (bucket_size * entry_size) is equal to the size of one bucket in bit
		 * so (bucket_index * bucket_size * entry_size) gives the index of the first bit of bucket bucket_index
		 */
		unsigned int const bit_index = bucket_index * bucket_size * entry_size + entry_index * entry_size;
		unsigned int const last_bit_index = bit_index + entry_size;
		assert(bit_index >= 0 && bit_index < bucket_count * bucket_size * entry_size);
		assert(last_bit_index >= 0 && last_bit_index <= bucket_count * bucket_size * entry_size);
		assert(bit_index <= last_bit_index);

		for(int count = 0, bi = bit_index; bi < last_bit_index; ++bi, ++count){
			unsigned int value = ((fp & (1 << count)) != 0);
			set_bit(bi, value);
		}
	}
	int space_in_bucket(unsigned int const bucket_index){
		fingerprint_t tmp;
		for(int i = 0; i < bucket_size; ++i){
			tmp = get_entry(bucket_index, i);
			if(tmp == empty_value)
				return i;
		}
		return -1; //return index available, -1 otherwise
	}
	bool search(element_type const* e, unsigned int& bucket_index, unsigned int& entry_index) const{
		fingerprint_t fp = funct::fingerprint(e);
		unsigned int h[2];
		h[0] = funct::hash(e);
		h[1] = h[0] ^ funct::hash(fp);
		for(int i = 0; i < 2; ++i){
			for(int j = 0; j < bucket_size; ++j){
				fingerprint_t tmp = get_entry(h[i], j);
				if(tmp == fp){
					bucket_index = h[i];
					entry_index = j;
					return true;
				}
			}
		}
		return false;
	}
	public:
	CuckooFilter(){
	}
	bool add(element_type const e){
		return add(&e);
	}
	bool add(element_type const* e){
		fingerprint_t fp = funct::fingerprint(e);
		unsigned int h1 = funct::hash(e);
		unsigned int h2 = h1 ^ funct::hash(fp);
		bool inserted = false;
		int i = 0;
		do{
			int space_h1 = space_in_bucket(h1);
			int space_h2 = space_in_bucket(h2);
			if(space_h1 < 0 && space_h2 >= 0){
				set_entry(h2, space_h2, fp);
				inserted = true;
				break;
			}
			else if(space_h1 >= 0 && space_h2 < 0){
				set_entry(h1, space_h1, fp);
				inserted = true;
				break;
			}
			else if(space_h1 >= 0 && space_h2 >= 0){
				double a = random_function();
				auto h = a > 0.5 ? h1 : h2;
				auto space_h = a > 0.5 ? space_h1 : space_h2;
				set_entry(h, space_h, fp);
				inserted = true;
				break;
			}
			else{
				double a = random_function();
				auto h = a > 0.5 ? h1 : h2;
				auto space_h = static_cast<unsigned int>(floor(random_function() * (bucket_size-1)));
				fingerprint_t tmp = get_entry(h, space_h);
				set_entry(h, space_h, fp);
				fp = tmp;
				h1 = h;
				h2 = h1 ^ funct::hash(fp);
			}
			i += 1;
		}while(i < 500);

		return inserted;
	}
	bool lookup(element_type const e){
		return lookup(&e);
	}
	bool lookup(element_type const* e){
		unsigned int bucket_index, entry_index;
		return search(e, bucket_index, entry_index);
	}
	void remove(element_type const e){
		remove(&e);
	}
	void remove(element_type const* e){
		unsigned int bucket_index, entry_index;
		bool ret = search(e, bucket_index, entry_index);
		if(ret){
			set_entry(bucket_index, entry_index, empty_value);
		}
	}
	void clear(void){
		for(unsigned char& byte : filter)
			byte = 0;
	}
};
