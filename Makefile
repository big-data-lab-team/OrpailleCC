EXE=main
TEST_DIR=./tests
SRC_DIR=./src
OBJECT=
CPPOBJECT=$(TEST_DIR)/test_bloom.oo\
		  $(TEST_DIR)/test_cuckoo.oo\
		  $(TEST_DIR)/test_reservoir_sampling.oo\
		  $(TEST_DIR)/test_chained_reservoir.oo\
		  $(TEST_DIR)/test_ltc.oo\
		  $(TEST_DIR)/test_mc-nn.oo 
FLAGS=-g
FLAGS_PERF=-O3

all: $(OBJECT) main.cpp
	g++ -I$(SRC_DIR) -std=c++11 main.cpp $(OBJECT) $(FLAGS) -o $(EXE)

test: $(CPPOBJECT) $(TEST_DIR)/test.cpp
	g++ -I$(SRC_DIR) -std=c++11 -fpermissive $(TEST_DIR)/test.cpp $(CPPOBJECT) $(FLAGS) -o $(EXE)-test -lgtest -lpthread

perf: $(OBJECT)
	g++ -I$(SRC_DIR) -std=c++11 main-performance.cpp $(OBJECT) $(FLAGS_PERF) -o $(EXE)-perf

run_test: test
	./$(EXE)_test

%.o: %.c
	gcc -std=c99 $< -c -o $@

%.oo: %.cpp
	g++ -I$(SRC_DIR) -std=c++11 $(FLAGS) $< -c -o $@ -fpermissive

clean:
	rm -f *.o *.oo $(TEST_DIR)/*.oo $(SRC_DIR)/*.oo $(EXE) $(EXE)-test $(EXE)-perf
