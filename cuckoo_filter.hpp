#include <cassert>
#include <type_traits>

#include <iostream>
#include <utility>

#define BYTE_SIZE (sizeof(unsigned char))
template<class element_type, int bucket_count, int bucket_size, int entry_size, typename hash, typename fingerprint>
class CuckooFilter{
	//Typedef to simplify the use of the hash function given by the user.
	typedef unsigned char fingerprint_t;
	unsigned int const element_size = sizeof(element_type);


	fingerprint_t get_entry(unsigned int const bucket_index, unsigned int const entry_index) const{
		return 0;
	}
	void set_entry(unsigned int const bucket_index, unsigned int const entry_index, fingerprint_t const fp){
	}
	public:
	CuckooFilter(){
	}
	bool add(element_type const e){
		return add(&e);
	}
	bool add(element_type const* e){
		return false;
	}
	bool lookup(element_type const e){
		return lookup(&e);
	}
	bool lookup(element_type const* e){
		return false;
	}
	void remove(element_type const e){
	}
	void remove(element_type const* e){
	}
	void clear(void){
	}
};
