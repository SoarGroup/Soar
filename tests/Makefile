OPTS=-I../src -I../src/models -L/home/jzxu/local/lib

all: lwrtest balltree_test floatvec_test

lwrtest: ../src/models/lwr.cpp ../src/common.cpp lwrtest.cpp 
	g++ -g -o lwrtest $(OPTS) $^ -larmadillo

balltree_test: ../src/models/balltree.cpp balltree_test.cpp
	g++ -pg -o $@ $(OPTS) $^ -larmadillo

clean:
	rm -f lwrtest balltree_test floatvec_test
	
