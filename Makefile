EXE=main
OBJECT=
CPPOBJECT=test_bloom.oo test_cuckoo.oo test_reservoir_sampling.cpp test_chained_reservoir.cpp test_mc-nn.cpp #heap.oo cuckoo.oo 
FLAGS=-g

all: $(OBJECT) main.cpp
	g++ -std=c++11 main.cpp $(OBJECT) $(FLAGS) -o $(EXE)

test: $(CPPOBJECT) test.cpp
	g++ -std=c++11  -fpermissive test.cpp $(CPPOBJECT) $(FLAGS)  -o $(EXE)_test -lgtest -lpthread

run_test: test
	./$(EXE)_test

%.o: %.c
	gcc -std=c99 $< -c -o $@

%.oo: %.cpp
	g++ -std=c++11 $(FLAGS) $< -c -o $@ -fpermissive

clean:
	rm -f *.o *.oo $(EXE) $(EXE)_test
