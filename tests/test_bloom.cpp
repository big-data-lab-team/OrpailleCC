#include "gtest/gtest.h"
#include "bloom_filter.hpp"

#define BLOOM_FILTER_SIZE 25

unsigned int hash_int(int* element){
	return (*element)%BLOOM_FILTER_SIZE;
}

TEST(BloomFilter, Add) { 
	auto p = hash_int;
	BloomFilter<int, BLOOM_FILTER_SIZE, 1> bf(&p);
	bf.add(3);
	EXPECT_EQ (true, bf.lookup(3));
	EXPECT_EQ (false, bf.lookup(4));
}

TEST(BloomFilter, false_positive) { 
	auto p = hash_int;
	BloomFilter<int, BLOOM_FILTER_SIZE, 1> bf(&p);
	bf.add(3);
	EXPECT_EQ (true, bf.lookup(3));
	EXPECT_EQ (true, bf.lookup((BLOOM_FILTER_SIZE + 3)));
	EXPECT_EQ (false, bf.lookup((BLOOM_FILTER_SIZE + 4)));
}
TEST(BloomFilter, clear) { 
	auto p = hash_int;
	BloomFilter<int, BLOOM_FILTER_SIZE, 1> bf(&p);
	EXPECT_EQ (false, bf.lookup(3));
	bf.add(3);
	EXPECT_EQ (true, bf.lookup(3));
	bf.clear();
	EXPECT_EQ (false, bf.lookup(3));
}
