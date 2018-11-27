#include "gtest/gtest.h"
#include "ltc.hpp"


TEST(LTC, Basic) { 
	LTC<3> comp;
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
