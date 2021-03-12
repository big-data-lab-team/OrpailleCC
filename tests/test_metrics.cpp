#include "gtest/gtest.h"
#include "metrics.hpp"

TEST(KappaMetrics, score) { 
	KappaMetrics<3> metric;
	metric.update(0, 0);
	metric.update(0, 0);
	metric.update(1, 1);
	metric.update(1, 1);
	metric.update(2, 2);
	metric.update(2, 2);
	metric.update(1, 0);
	metric.update(0, 1);
	metric.update(1, 2);
	ASSERT_NEAR(metric.kappa(), 0.5, 0.0001);
	KappaMetrics<34> metric1;
	metric1.update(0,0);
	metric1.update(9,33);
	metric1.update(33,33);
	metric1.update(15,4);
	metric1.update(22,22);
	metric1.update(6,23);
	metric1.update(29,29);
	metric1.update(1,1);
	metric1.update(33,33);
	metric1.update(20,29);
	metric1.update(21,21);
	metric1.update(22,18);
	metric1.update(2,2);
	metric1.update(18,3);
	metric1.update(15,15);
	metric1.update(7,3);
	metric1.update(18,18);
	metric1.update(33,7);
	metric1.update(19,19);
	metric1.update(14,29);
	metric1.update(18,18);
	metric1.update(2,30);
	metric1.update(29,29);
	metric1.update(19,26);
	metric1.update(2,2);
	metric1.update(28,12);
	metric1.update(1,1);
	metric1.update(28,27);
	metric1.update(21,21);
	metric1.update(28,17);
	metric1.update(29,29);
	metric1.update(18,0);
	metric1.update(8,8);
	metric1.update(11,6);
	metric1.update(26,26);
	metric1.update(14,6);
	metric1.update(29,29);
	metric1.update(23,20);
	metric1.update(0,0);
	metric1.update(25,33);
	metric1.update(14,14);
	metric1.update(29,25);
	metric1.update(17,17);
	metric1.update(3,6);
	metric1.update(32,32);
	metric1.update(21,25);
	metric1.update(24,24);
	metric1.update(21,28);
	metric1.update(24,24);
	metric1.update(26,11);
	metric1.update(30,30);
	metric1.update(16,21);
	metric1.update(8,8);
	metric1.update(17,5);
	metric1.update(11,11);
	metric1.update(31,19);
	metric1.update(1,1);
	metric1.update(17,24);
	metric1.update(23,23);
	metric1.update(20,25);
	metric1.update(22,22);
	metric1.update(5,24);
	metric1.update(17,17);
	metric1.update(15,26);
	metric1.update(28,28);
	metric1.update(22,13);
	metric1.update(5,5);
	metric1.update(14,2);
	metric1.update(10,10);
	metric1.update(9,14);
	metric1.update(9,9);
	metric1.update(14,7);
	metric1.update(24,24);
	metric1.update(13,29);
	metric1.update(25,25);
	metric1.update(33,5);
	metric1.update(26,26);
	metric1.update(29,33);
	metric1.update(8,8);
	metric1.update(0,4);
	metric1.update(7,7);
	metric1.update(18,32);
	metric1.update(31,31);
	metric1.update(29,30);
	metric1.update(11,11);
	metric1.update(6,6);
	metric1.update(18,18);
	metric1.update(27,19);
	metric1.update(32,32);
	metric1.update(25,10);
	metric1.update(12,12);
	metric1.update(14,17);
	metric1.update(5,5);
	metric1.update(28,14);
	metric1.update(28,28);
	metric1.update(6,29);
	metric1.update(23,23);
	metric1.update(8,19);
	metric1.update(28,28);
	metric1.update(21,20);
	ASSERT_NEAR(metric1.kappa(), 0.5026937422295898, 0.0001);
	KappaMetrics<34> metric2;
	metric2.update(0,1);
	metric2.update(9,10);
	metric2.update(33,1);
	metric2.update(15,16);
	metric2.update(22,23);
	metric2.update(6,7);
	metric2.update(29,30);
	metric2.update(1,2);
	metric2.update(33,1);
	metric2.update(20,21);
	metric2.update(21,22);
	metric2.update(22,23);
	metric2.update(2,3);
	metric2.update(18,19);
	metric2.update(15,16);
	metric2.update(7,8);
	metric2.update(18,19);
	metric2.update(33,1);
	metric2.update(19,20);
	metric2.update(14,15);
	metric2.update(18,19);
	metric2.update(2,3);
	metric2.update(29,30);
	metric2.update(19,20);
	metric2.update(2,3);
	metric2.update(28,29);
	metric2.update(1,2);
	metric2.update(28,29);
	metric2.update(21,22);
	metric2.update(28,29);
	metric2.update(29,30);
	metric2.update(18,19);
	metric2.update(8,9);
	metric2.update(11,12);
	metric2.update(26,27);
	metric2.update(14,15);
	metric2.update(29,30);
	metric2.update(23,24);
	metric2.update(0,1);
	metric2.update(25,26);
	metric2.update(14,15);
	metric2.update(29,30);
	metric2.update(17,18);
	metric2.update(3,4);
	metric2.update(32,0);
	metric2.update(21,22);
	metric2.update(24,25);
	metric2.update(21,22);
	metric2.update(24,25);
	metric2.update(26,27);
	metric2.update(30,31);
	metric2.update(16,17);
	metric2.update(8,9);
	metric2.update(17,18);
	metric2.update(11,12);
	metric2.update(31,32);
	metric2.update(1,2);
	metric2.update(17,18);
	metric2.update(23,24);
	metric2.update(20,21);
	metric2.update(22,23);
	metric2.update(5,6);
	metric2.update(17,18);
	metric2.update(15,16);
	metric2.update(28,29);
	metric2.update(22,23);
	metric2.update(5,6);
	metric2.update(14,15);
	metric2.update(10,11);
	metric2.update(9,10);
	metric2.update(9,10);
	metric2.update(14,15);
	metric2.update(24,25);
	metric2.update(13,14);
	metric2.update(25,26);
	metric2.update(33,1);
	metric2.update(26,27);
	metric2.update(29,30);
	metric2.update(8,9);
	metric2.update(0,1);
	metric2.update(7,8);
	metric2.update(18,19);
	metric2.update(31,32);
	metric2.update(29,30);
	metric2.update(11,12);
	metric2.update(6,7);
	metric2.update(18,19);
	metric2.update(27,28);
	metric2.update(32,0);
	metric2.update(25,26);
	metric2.update(12,13);
	metric2.update(14,15);
	metric2.update(5,6);
	metric2.update(28,29);
	metric2.update(28,29);
	metric2.update(6,7);
	metric2.update(23,24);
	metric2.update(8,9);
	metric2.update(28,29);
	metric2.update(21,22);
	ASSERT_NEAR(metric2.kappa(), -0.030502885408079328, 0.0001);
}
TEST(KappaMetrics, reset) { 
	KappaMetrics<34> metric;
	metric.update(0,0);
	metric.update(9,33);
	metric.update(33,33);
	metric.update(15,4);
	metric.update(22,22);
	metric.update(6,23);
	metric.update(29,29);
	metric.update(1,1);
	metric.update(33,33);
	metric.update(20,29);
	metric.update(21,21);
	metric.update(22,18);
	metric.update(2,2);
	metric.update(18,3);
	metric.update(15,15);
	metric.update(7,3);
	metric.update(18,18);
	metric.update(33,7);
	metric.update(19,19);
	metric.update(14,29);
	metric.update(18,18);
	metric.update(2,30);
	metric.update(29,29);
	metric.update(19,26);
	metric.update(2,2);
	metric.update(28,12);
	metric.update(1,1);
	metric.update(28,27);
	metric.update(21,21);
	metric.update(28,17);
	metric.update(29,29);
	metric.update(18,0);
	metric.update(8,8);
	metric.update(11,6);
	metric.update(26,26);
	metric.update(14,6);
	metric.update(29,29);
	metric.update(23,20);
	metric.update(0,0);
	metric.update(25,33);
	metric.update(14,14);
	metric.update(29,25);
	metric.update(17,17);
	metric.update(3,6);
	metric.update(32,32);
	metric.update(21,25);
	metric.update(24,24);
	metric.update(21,28);
	metric.update(24,24);
	metric.update(26,11);
	metric.update(30,30);
	metric.update(16,21);
	metric.update(8,8);
	metric.update(17,5);
	metric.update(11,11);
	metric.update(31,19);
	metric.update(1,1);
	metric.update(17,24);
	metric.update(23,23);
	metric.update(20,25);
	metric.update(22,22);
	metric.update(5,24);
	metric.update(17,17);
	metric.update(15,26);
	metric.update(28,28);
	metric.update(22,13);
	metric.update(5,5);
	metric.update(14,2);
	metric.update(10,10);
	metric.update(9,14);
	metric.update(9,9);
	metric.update(14,7);
	metric.update(24,24);
	metric.update(13,29);
	metric.update(25,25);
	metric.update(33,5);
	metric.update(26,26);
	metric.update(29,33);
	metric.update(8,8);
	metric.update(0,4);
	metric.update(7,7);
	metric.update(18,32);
	metric.update(31,31);
	metric.update(29,30);
	metric.update(11,11);
	metric.update(6,6);
	metric.update(18,18);
	metric.update(27,19);
	metric.update(32,32);
	metric.update(25,10);
	metric.update(12,12);
	metric.update(14,17);
	metric.update(5,5);
	metric.update(28,14);
	metric.update(28,28);
	metric.update(6,29);
	metric.update(23,23);
	metric.update(8,19);
	metric.update(28,28);
	metric.update(21,20);
	ASSERT_NEAR(metric.kappa(), 0.5026937422295898, 0.0001);
	metric.reset();
	metric.update(0,1);
	metric.update(9,10);
	metric.update(33,1);
	metric.update(15,16);
	metric.update(22,23);
	metric.update(6,7);
	metric.update(29,30);
	metric.update(1,2);
	metric.update(33,1);
	metric.update(20,21);
	metric.update(21,22);
	metric.update(22,23);
	metric.update(2,3);
	metric.update(18,19);
	metric.update(15,16);
	metric.update(7,8);
	metric.update(18,19);
	metric.update(33,1);
	metric.update(19,20);
	metric.update(14,15);
	metric.update(18,19);
	metric.update(2,3);
	metric.update(29,30);
	metric.update(19,20);
	metric.update(2,3);
	metric.update(28,29);
	metric.update(1,2);
	metric.update(28,29);
	metric.update(21,22);
	metric.update(28,29);
	metric.update(29,30);
	metric.update(18,19);
	metric.update(8,9);
	metric.update(11,12);
	metric.update(26,27);
	metric.update(14,15);
	metric.update(29,30);
	metric.update(23,24);
	metric.update(0,1);
	metric.update(25,26);
	metric.update(14,15);
	metric.update(29,30);
	metric.update(17,18);
	metric.update(3,4);
	metric.update(32,0);
	metric.update(21,22);
	metric.update(24,25);
	metric.update(21,22);
	metric.update(24,25);
	metric.update(26,27);
	metric.update(30,31);
	metric.update(16,17);
	metric.update(8,9);
	metric.update(17,18);
	metric.update(11,12);
	metric.update(31,32);
	metric.update(1,2);
	metric.update(17,18);
	metric.update(23,24);
	metric.update(20,21);
	metric.update(22,23);
	metric.update(5,6);
	metric.update(17,18);
	metric.update(15,16);
	metric.update(28,29);
	metric.update(22,23);
	metric.update(5,6);
	metric.update(14,15);
	metric.update(10,11);
	metric.update(9,10);
	metric.update(9,10);
	metric.update(14,15);
	metric.update(24,25);
	metric.update(13,14);
	metric.update(25,26);
	metric.update(33,1);
	metric.update(26,27);
	metric.update(29,30);
	metric.update(8,9);
	metric.update(0,1);
	metric.update(7,8);
	metric.update(18,19);
	metric.update(31,32);
	metric.update(29,30);
	metric.update(11,12);
	metric.update(6,7);
	metric.update(18,19);
	metric.update(27,28);
	metric.update(32,0);
	metric.update(25,26);
	metric.update(12,13);
	metric.update(14,15);
	metric.update(5,6);
	metric.update(28,29);
	metric.update(28,29);
	metric.update(6,7);
	metric.update(23,24);
	metric.update(8,9);
	metric.update(28,29);
	metric.update(21,22);
	ASSERT_NEAR(metric.kappa(), -0.030502885408079328, 0.0001);
}
TEST(ErrorMetrics, score) { 
	ErrorMetrics metric;
	metric.update(0, 0);
	metric.update(0, 0);
	metric.update(1, 1);
	metric.update(1, 1);
	metric.update(2, 2);
	metric.update(2, 2);
	metric.update(1, 0);
	metric.update(0, 1);
	metric.update(1, 2);
	metric.update(0, 2);
	ASSERT_NEAR(metric.score(), 0.4, 0.0001);

	ErrorMetrics metric1;
	metric1.update(0,1);
	metric1.update(9,10);
	metric1.update(33,1);
	metric1.update(15,16);
	metric1.update(22,23);
	metric1.update(6,7);
	metric1.update(29,30);
	metric1.update(1,2);
	metric1.update(33,1);
	metric1.update(20,21);
	metric1.update(21,22);
	metric1.update(22,23);
	metric1.update(2,3);
	metric1.update(18,19);
	metric1.update(15,16);
	metric1.update(7,8);
	metric1.update(18,19);
	metric1.update(33,1);
	metric1.update(19,20);
	metric1.update(14,15);
	metric1.update(18,19);
	metric1.update(2,3);
	metric1.update(29,30);
	metric1.update(19,20);
	metric1.update(2,3);
	metric1.update(28,29);
	metric1.update(1,2);
	metric1.update(28,29);
	metric1.update(21,22);
	metric1.update(28,29);
	metric1.update(29,30);
	metric1.update(18,19);
	metric1.update(8,9);
	metric1.update(11,12);
	metric1.update(26,27);
	metric1.update(14,15);
	metric1.update(29,30);
	metric1.update(23,24);
	metric1.update(0,1);
	metric1.update(25,26);
	metric1.update(14,15);
	metric1.update(29,30);
	metric1.update(17,18);
	metric1.update(3,4);
	metric1.update(32,0);
	metric1.update(21,22);
	metric1.update(24,25);
	metric1.update(21,22);
	metric1.update(24,25);
	metric1.update(26,27);
	metric1.update(30,31);
	metric1.update(16,17);
	metric1.update(8,9);
	metric1.update(17,18);
	metric1.update(11,12);
	metric1.update(31,32);
	metric1.update(1,2);
	metric1.update(17,18);
	metric1.update(23,24);
	metric1.update(20,21);
	metric1.update(22,23);
	metric1.update(5,6);
	metric1.update(17,18);
	metric1.update(15,16);
	metric1.update(28,29);
	metric1.update(22,23);
	metric1.update(5,6);
	metric1.update(14,15);
	metric1.update(10,11);
	metric1.update(9,10);
	metric1.update(9,10);
	metric1.update(14,15);
	metric1.update(24,25);
	metric1.update(13,14);
	metric1.update(25,26);
	metric1.update(33,1);
	metric1.update(26,27);
	metric1.update(29,30);
	metric1.update(8,9);
	metric1.update(0,1);
	metric1.update(7,8);
	metric1.update(18,19);
	metric1.update(31,32);
	metric1.update(29,30);
	metric1.update(11,12);
	metric1.update(6,7);
	metric1.update(18,19);
	metric1.update(27,28);
	metric1.update(32,0);
	metric1.update(25,26);
	metric1.update(12,13);
	metric1.update(14,15);
	metric1.update(5,6);
	metric1.update(28,29);
	metric1.update(28,29);
	metric1.update(6,7);
	metric1.update(23,24);
	metric1.update(8,9);
	metric1.update(28,29);
	metric1.update(21,22);
	ASSERT_NEAR(metric1.score(), 1.0, 0.00001);

	ErrorMetrics metric2;
	metric2.update(0,0);
	metric2.update(9,18);
	metric2.update(33,33);
	metric2.update(15,31);
	metric2.update(22,22);
	metric2.update(6,30);
	metric2.update(29,29);
	metric2.update(1,18);
	metric2.update(33,33);
	metric2.update(20,31);
	metric2.update(21,21);
	metric2.update(22,31);
	metric2.update(2,2);
	metric2.update(18,3);
	metric2.update(15,15);
	metric2.update(7,17);
	metric2.update(18,18);
	metric2.update(33,9);
	metric2.update(19,19);
	metric2.update(14,1);
	metric2.update(18,18);
	metric2.update(2,16);
	metric2.update(29,29);
	metric2.update(19,2);
	metric2.update(2,2);
	metric2.update(28,14);
	metric2.update(1,1);
	metric2.update(28,30);
	metric2.update(21,21);
	metric2.update(28,10);
	metric2.update(29,29);
	metric2.update(18,20);
	metric2.update(8,8);
	metric2.update(11,32);
	metric2.update(26,26);
	metric2.update(14,15);
	metric2.update(29,29);
	metric2.update(23,14);
	metric2.update(0,0);
	metric2.update(25,20);
	metric2.update(14,14);
	metric2.update(29,4);
	metric2.update(17,17);
	metric2.update(3,16);
	metric2.update(32,32);
	metric2.update(21,33);
	metric2.update(24,24);
	metric2.update(21,12);
	metric2.update(24,24);
	metric2.update(26,19);
	metric2.update(30,30);
	metric2.update(16,30);
	metric2.update(8,8);
	metric2.update(17,11);
	metric2.update(11,11);
	metric2.update(31,22);
	metric2.update(1,1);
	metric2.update(17,21);
	metric2.update(23,23);
	metric2.update(20,25);
	metric2.update(22,22);
	metric2.update(5,12);
	metric2.update(17,17);
	metric2.update(15,19);
	metric2.update(28,28);
	metric2.update(22,29);
	metric2.update(5,5);
	metric2.update(14,6);
	metric2.update(10,10);
	metric2.update(9,21);
	metric2.update(9,9);
	metric2.update(14,28);
	metric2.update(24,24);
	metric2.update(13,2);
	metric2.update(25,25);
	metric2.update(33,2);
	metric2.update(26,26);
	metric2.update(29,14);
	metric2.update(8,8);
	metric2.update(0,32);
	metric2.update(7,7);
	metric2.update(18,2);
	metric2.update(31,31);
	metric2.update(29,2);
	metric2.update(11,11);
	metric2.update(6,13);
	metric2.update(18,18);
	metric2.update(27,0);
	metric2.update(32,32);
	metric2.update(25,17);
	metric2.update(12,12);
	metric2.update(14,16);
	metric2.update(5,5);
	metric2.update(28,26);
	metric2.update(28,28);
	metric2.update(6,16);
	metric2.update(23,23);
	metric2.update(8,13);
	metric2.update(28,28);
	metric2.update(21,21);
	ASSERT_NEAR(metric2.score(), 0.49, 0.00001);
}
TEST(ErrorMetrics, reset) { 
	ErrorMetrics metric;
	metric.update(0,1);
	metric.update(9,10);
	metric.update(33,1);
	metric.update(15,16);
	metric.update(22,23);
	metric.update(6,7);
	metric.update(29,30);
	metric.update(1,2);
	metric.update(33,1);
	metric.update(20,21);
	metric.update(21,22);
	metric.update(22,23);
	metric.update(2,3);
	metric.update(18,19);
	metric.update(15,16);
	metric.update(7,8);
	metric.update(18,19);
	metric.update(33,1);
	metric.update(19,20);
	metric.update(14,15);
	metric.update(18,19);
	metric.update(2,3);
	metric.update(29,30);
	metric.update(19,20);
	metric.update(2,3);
	metric.update(28,29);
	metric.update(1,2);
	metric.update(28,29);
	metric.update(21,22);
	metric.update(28,29);
	metric.update(29,30);
	metric.update(18,19);
	metric.update(8,9);
	metric.update(11,12);
	metric.update(26,27);
	metric.update(14,15);
	metric.update(29,30);
	metric.update(23,24);
	metric.update(0,1);
	metric.update(25,26);
	metric.update(14,15);
	metric.update(29,30);
	metric.update(17,18);
	metric.update(3,4);
	metric.update(32,0);
	metric.update(21,22);
	metric.update(24,25);
	metric.update(21,22);
	metric.update(24,25);
	metric.update(26,27);
	metric.update(30,31);
	metric.update(16,17);
	metric.update(8,9);
	metric.update(17,18);
	metric.update(11,12);
	metric.update(31,32);
	metric.update(1,2);
	metric.update(17,18);
	metric.update(23,24);
	metric.update(20,21);
	metric.update(22,23);
	metric.update(5,6);
	metric.update(17,18);
	metric.update(15,16);
	metric.update(28,29);
	metric.update(22,23);
	metric.update(5,6);
	metric.update(14,15);
	metric.update(10,11);
	metric.update(9,10);
	metric.update(9,10);
	metric.update(14,15);
	metric.update(24,25);
	metric.update(13,14);
	metric.update(25,26);
	metric.update(33,1);
	metric.update(26,27);
	metric.update(29,30);
	metric.update(8,9);
	metric.update(0,1);
	metric.update(7,8);
	metric.update(18,19);
	metric.update(31,32);
	metric.update(29,30);
	metric.update(11,12);
	metric.update(6,7);
	metric.update(18,19);
	metric.update(27,28);
	metric.update(32,0);
	metric.update(25,26);
	metric.update(12,13);
	metric.update(14,15);
	metric.update(5,6);
	metric.update(28,29);
	metric.update(28,29);
	metric.update(6,7);
	metric.update(23,24);
	metric.update(8,9);
	metric.update(28,29);
	metric.update(21,22);
	ASSERT_NEAR(metric.score(), 1.0, 0.00001);

	metric.reset();
	metric.update(0,0);
	metric.update(9,18);
	metric.update(33,33);
	metric.update(15,31);
	metric.update(22,22);
	metric.update(6,30);
	metric.update(29,29);
	metric.update(1,18);
	metric.update(33,33);
	metric.update(20,31);
	metric.update(21,21);
	metric.update(22,31);
	metric.update(2,2);
	metric.update(18,3);
	metric.update(15,15);
	metric.update(7,17);
	metric.update(18,18);
	metric.update(33,9);
	metric.update(19,19);
	metric.update(14,1);
	metric.update(18,18);
	metric.update(2,16);
	metric.update(29,29);
	metric.update(19,2);
	metric.update(2,2);
	metric.update(28,14);
	metric.update(1,1);
	metric.update(28,30);
	metric.update(21,21);
	metric.update(28,10);
	metric.update(29,29);
	metric.update(18,20);
	metric.update(8,8);
	metric.update(11,32);
	metric.update(26,26);
	metric.update(14,15);
	metric.update(29,29);
	metric.update(23,14);
	metric.update(0,0);
	metric.update(25,20);
	metric.update(14,14);
	metric.update(29,4);
	metric.update(17,17);
	metric.update(3,16);
	metric.update(32,32);
	metric.update(21,33);
	metric.update(24,24);
	metric.update(21,12);
	metric.update(24,24);
	metric.update(26,19);
	metric.update(30,30);
	metric.update(16,30);
	metric.update(8,8);
	metric.update(17,11);
	metric.update(11,11);
	metric.update(31,22);
	metric.update(1,1);
	metric.update(17,21);
	metric.update(23,23);
	metric.update(20,25);
	metric.update(22,22);
	metric.update(5,12);
	metric.update(17,17);
	metric.update(15,19);
	metric.update(28,28);
	metric.update(22,29);
	metric.update(5,5);
	metric.update(14,6);
	metric.update(10,10);
	metric.update(9,21);
	metric.update(9,9);
	metric.update(14,28);
	metric.update(24,24);
	metric.update(13,2);
	metric.update(25,25);
	metric.update(33,2);
	metric.update(26,26);
	metric.update(29,14);
	metric.update(8,8);
	metric.update(0,32);
	metric.update(7,7);
	metric.update(18,2);
	metric.update(31,31);
	metric.update(29,2);
	metric.update(11,11);
	metric.update(6,13);
	metric.update(18,18);
	metric.update(27,0);
	metric.update(32,32);
	metric.update(25,17);
	metric.update(12,12);
	metric.update(14,16);
	metric.update(5,5);
	metric.update(28,26);
	metric.update(28,28);
	metric.update(6,16);
	metric.update(23,23);
	metric.update(8,13);
	metric.update(28,28);
	metric.update(21,21);
	ASSERT_NEAR(metric.score(), 0.49, 0.00001);
}
TEST(ReservoirSamplingMetrics, score) { 

	int const sample_size = 10;
	ReservoirSamplingMetrics sample[sample_size+1];
	

	for(int j = 0; j < 10; ++j){
		double sum_proba = 0;
		for(int i = 0; i < sample_size+1; ++i)
			sum_proba += sample[i].score(false, sample_size);

		ASSERT_NEAR(sum_proba, 1.0, 0.00001);

		double const score_last_element = sample[sample_size].score(false, sample_size);
		double const score_sample_element = sample[0].score(false, sample_size);

		ASSERT_TRUE(score_last_element >= score_sample_element);
	
		//Advance the last element by 77.
		for(int i = 0; i < 77; ++i)
			sample[sample_size-1].reset();
	}
}
