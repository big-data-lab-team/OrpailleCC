EXE=main
OBJECT=
CPPOBJECT=test_bloom.oo test_cuckoo.oo test_reservoir_sampling.cpp #heap.oo cuckoo.oo 

all: $(OBJECT)
	g++ -std=c++11 main.cpp $(OBJECT) -o $(EXE)

test: $(CPPOBJECT)
	g++ -std=c++11  -fpermissive test.cpp $(CPPOBJECT) -o $(EXE)_test -lgtest -lpthread

run_test: test
	./$(EXE)_test

%.o: %.c
	gcc -std=c99 $< -c -o $@

%.oo: %.cpp
	g++ -std=c++11 $< -c -o $@ -fpermissive

clean:
	rm -f *.o *.oo $(EXE) $(EXE)_test
