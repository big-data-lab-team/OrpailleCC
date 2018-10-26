#include "gtest/gtest.h"
#include "cuckoo_filter.hpp"

#define CUCKOO_BUCKET_COUNT 25
#define CUCKOO_ENTRY_BY_BUCKET 3
#define CUCKOO_ENTRY_SIZE 4
struct hash1{
	unsigned int operator()(int* e){
		return (*e)%CUCKOO_BUCKET_COUNT;		
	}
	unsigned int operator()(unsigned char fingerprint){
		return (fingerprint * 7)%CUCKOO_BUCKET_COUNT;
	}
};
struct finger1{
	unsigned char operator()(int* e){
		return (*e)%16; //2^4
	}
};

TEST(CuckooFilter, Add) { 
	CuckooFilter<int, CUCKOO_BUCKET_COUNT, CUCKOO_ENTRY_BY_BUCKET, CUCKOO_ENTRY_SIZE, hash1, finger1> cf;
	cf.add(3);
	EXPECT_EQ (false, cf.lookup(2));
	EXPECT_EQ (true , cf.lookup(3));
	EXPECT_EQ (false, cf.lookup(4));
}

TEST(CuckooFilter, remove) { 
	CuckooFilter<int, CUCKOO_BUCKET_COUNT, CUCKOO_ENTRY_BY_BUCKET, CUCKOO_ENTRY_SIZE, hash1, finger1> cf;
	for(int i = 0; i < 500; ++i){
		cf.add(i);
		EXPECT_EQ (true , cf.lookup(i));
		cf.remove(i);
		EXPECT_EQ (false , cf.lookup(i));
	}
}
TEST(CuckooFilter, full) { 
	CuckooFilter<int, CUCKOO_BUCKET_COUNT, CUCKOO_ENTRY_BY_BUCKET, CUCKOO_ENTRY_SIZE, hash1, finger1> cf;
	bool full = false;	
	for(int i = 0; i < 500 && !full; ++i){
		bool res = cf.add(i);
		EXPECT_EQ (true , cf.lookup(i));
		full = full || res;
	}
	EXPECT_EQ (true , full);
}

