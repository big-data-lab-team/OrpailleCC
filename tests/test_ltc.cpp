#include <vector>
#include <cmath>
#include "gtest/gtest.h"
#include "ltc.hpp"


TEST(LTC, Basic) { 
	LTC<int, int, 3> comp;
	int first_value;
	for(int i = 0; i < 20; ++i){
		int const val = 10 + ((rand()%3) - 1);
		if(i == 0)
			first_value = val;
		bool a = comp.add(i, val);
		EXPECT_EQ (false, a);
	}
	EXPECT_EQ (true, comp.add(20, 20));
	int timestamp, value;
	comp.get_value_to_transmit(timestamp, value);
	EXPECT_EQ (0, timestamp);
	EXPECT_EQ (first_value, value);
}
TEST(LTC, Perfect) { 
	LTC<int, int, 3> comp;
	int count_compress = 0;
	for(int i = 0; i < 400; ++i){
		bool a = comp.add(i, i);
		if(a)
			count_compress += 1;
	}
	EXPECT_EQ (0, count_compress); //It is one line only, so ltc should never break the line
}
TEST(LTC, Linear) {
	LTC<int, int, 3> comp;
	int count_compress = 0;
	for(int i = 0; i < 1100; ++i){
		bool a = comp.add(i, i%200);
		if(a)
			count_compress += 1;
	}
	EXPECT_TRUE (count_compress < 30); //It is very linear, so I expect good compression
	EXPECT_TRUE (count_compress > 5); //There is 5 lines (1100 / 200)
}
TEST(LTC, Cos) { 
	LTC<int, int, 3> comp;
	int count_compress = 0;
	for(int i = 0; i < 1100; ++i){
		int val = round(cos(i*0.1) * 20);
		bool a = comp.add(i, val);
		if(a)
			count_compress += 1;
	}
	EXPECT_TRUE (count_compress < 1100); //Some compression is expected
	EXPECT_TRUE (count_compress > 4); //More than 4 because there is roughly 4 linear lines in this function
}
TEST(LTC, Compression_level) { 
	LTC<int, int, 3> comp3;
	LTC<int, int, 5> comp5;
	int count_3 = 0, count_5 = 0;
	for(int i = 0; i < 1100; ++i){
		int val = round(cos(i*0.1) * 20);
		bool a = comp3.add(i, val);
		if(a)
			count_3 += 1;
		a = comp5.add(i, val);
		if(a)
			count_5 += 1;
	}
	EXPECT_TRUE (count_3 < 1100); //Some compression is expected
	EXPECT_TRUE (count_3 > 4); //More than 4 because there is roughly 4 linear lines in this function
	EXPECT_TRUE (count_5 < 1100); //Some compression is expected
	EXPECT_TRUE (count_5 > 4); //More than 4 because there is roughly 4 linear lines in this function
	EXPECT_TRUE (count_5 < count_3); //comp5 has a better compression level
}
