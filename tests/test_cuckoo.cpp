#include "gtest/gtest.h"
#include "cuckoo_filter.hpp"

#define CUCKOO_BUCKET_COUNT 4
#define CUCKOO_ENTRY_BY_BUCKET 3
#define CUCKOO_ENTRY_SIZE 3
struct funct{
	static unsigned char fingerprint(int const* e){
		//mod 7 == value between 0 and 6, +1 == value between 1 and 7, so the empty value (0x0) is avoided
		return ((*e)%7)+1; 
	}
	static unsigned int hash(int const* e){
		return (*e)%CUCKOO_BUCKET_COUNT;		
	}
	/*
	 * Make the combination of hash element and hash fingerprint does not lead to the same value
	 * Hash of fingerprint should probably not return a 0 or a 2^CUCKOO_ENTRY_SIZE-1 because it create same h1 and h2
	 */
	static unsigned int hash(unsigned char fingerprint){
		return (fingerprint * 7)%CUCKOO_BUCKET_COUNT;
	}
};
double randy_cuckoo(void){
	return (double)rand() / (double)RAND_MAX;
}

TEST(CuckooFilter, Add) { 
	CuckooFilter<int, CUCKOO_BUCKET_COUNT, CUCKOO_ENTRY_BY_BUCKET, CUCKOO_ENTRY_SIZE, funct, randy_cuckoo> cf;
	cf.add(3);
	EXPECT_EQ (false, cf.lookup(2));
	EXPECT_EQ (true , cf.lookup(3));
	EXPECT_EQ (false, cf.lookup(4));
	cf.add(3);
	EXPECT_EQ (false, cf.lookup(2));
	EXPECT_EQ (true , cf.lookup(3));
	EXPECT_EQ (false, cf.lookup(4));
	cf.add(4);
	EXPECT_EQ (false, cf.lookup(2));
	EXPECT_EQ (true , cf.lookup(3));
	EXPECT_EQ (true, cf.lookup(4));
}

TEST(CuckooFilter, remove) { 
	CuckooFilter<int, CUCKOO_BUCKET_COUNT, CUCKOO_ENTRY_BY_BUCKET, CUCKOO_ENTRY_SIZE, funct, randy_cuckoo> cf;
	for(int i = 0; i < 500; ++i){
		cf.add(i);
		EXPECT_EQ (true , cf.lookup(i));
		cf.remove(i);
		EXPECT_EQ (false , cf.lookup(i));
	}
}
TEST(CuckooFilter, full) { 
	CuckooFilter<int, CUCKOO_BUCKET_COUNT, CUCKOO_ENTRY_BY_BUCKET, CUCKOO_ENTRY_SIZE, funct, randy_cuckoo> cf;
	bool full = false;	
	for(int i = 0; i < 500 && !full; ++i){
		bool res = cf.add(i);
		EXPECT_EQ (true , cf.lookup(i));
		full = full || res;
	}
	EXPECT_EQ (true , full);
}

struct funct_2{
	static unsigned char fingerprint(int const* e){
		//mod 127 == value between 0 and 126, +1 == value between 1 and 127, so the empty value (0x0) is avoided
		return ((*e)%127)+1; 
	}
	static unsigned int hash(int const* e){
		return (*e)%CUCKOO_BUCKET_COUNT;		
	}
	/*
	 * Make the combination of hash element and hash fingerprint does not lead to the same value
	 * Hash of fingerprint should probably not return a 0 or a 2^CUCKOO_ENTRY_SIZE-1 because it create same h1 and h2
	 */
	static unsigned int hash(unsigned char fingerprint){
		return (fingerprint * 7)%CUCKOO_BUCKET_COUNT;
	}
};
TEST(CuckooFilter, Add_7_bits) { 
	CuckooFilter<int, CUCKOO_BUCKET_COUNT, CUCKOO_ENTRY_BY_BUCKET, 7, funct_2, randy_cuckoo> cf;
	cf.add(3);
	EXPECT_EQ (false, cf.lookup(2));
	EXPECT_EQ (true , cf.lookup(3));
	EXPECT_EQ (false, cf.lookup(4));
	cf.add(3);
	EXPECT_EQ (false, cf.lookup(2));
	EXPECT_EQ (true , cf.lookup(3));
	EXPECT_EQ (false, cf.lookup(4));
	cf.add(4);
	EXPECT_EQ (false, cf.lookup(2));
	EXPECT_EQ (true , cf.lookup(3));
	EXPECT_EQ (true, cf.lookup(4));
}
TEST(CuckooFilter, remove_7_bits) { 
	CuckooFilter<int, CUCKOO_BUCKET_COUNT, CUCKOO_ENTRY_BY_BUCKET, 7, funct_2, randy_cuckoo> cf;
	for(int i = 0; i < 500; ++i){
		cf.add(i);
		EXPECT_EQ (true , cf.lookup(i));
		cf.remove(i);
		EXPECT_EQ (false , cf.lookup(i));
	}
}
TEST(CuckooFilter, full_7_bits) { 
	CuckooFilter<int, CUCKOO_BUCKET_COUNT, CUCKOO_ENTRY_BY_BUCKET, 7, funct_2, randy_cuckoo> cf;
	bool full = false;	
	for(int i = 0; i < 500 && !full; ++i){
		bool res = cf.add(i);
		EXPECT_EQ (true , cf.lookup(i));
		full = full || res;
	}
	EXPECT_EQ (true , full);
}
