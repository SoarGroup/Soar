SRC = ../models/em.cpp ../models/linear.cpp ../models/dtree.cpp ../sgnode.cpp ../scene.cpp ../drawer.cpp ../common.cpp ../filters/*.cpp ../filter_factory.cpp ../bullet_support.cpp
HDR = ../sgnode.h ../scene.h ../drawer.h ../common.h ../filter.h ../linalg.h ../bullet_support.h ../models/dtree.h
RHOME = /home/jzxu/R/i686-pc-linux-gnu-library/2.13
LIBPATH = -L/usr/lib/R/lib -L$(RHOME)/Rcpp/lib -L$(RHOME)/RInside/lib -L/home/jzxu/local/lib
INCPATH = -I.. -I../models/ -I/usr/include/R -I$(RHOME)/Rcpp/include -I$(RHOME)/RInside/include -I/usr/include/bullet -I/home/jzxu/local/include
LIBS = -lBulletCollision -lLinearMath -lR -lRcpp -lRInside -larmadillo -lblas -llapack
FLAGS = -Wfatal-errors -std=c++0x -g

all: em_test dtree_test

em_test: em_test.cpp $(SRC) all.h.gch
	g++ -include all.h $(FLAGS) -o em_test $(LIBPATH) $(INCPATH) em_test.cpp $(SRC) $(LIBS) 

dtree_test: dtree_test.cpp $(SRC) all.h.gch
	g++ -include all.h $(FLAGS) -o dtree_test $(LIBPATH) $(INCPATH) dtree_test.cpp $(SRC) $(LIBS)

all.h.gch: all.h $(HDR)
	g++ $(FLAGS) $(INCPATH) all.h
	
test: test.cpp
	g++ $(FLAGS) -o test $(LIBPATH) $(INCPATH) test.cpp $(LIBS)

clean:
	rm -f em_test dtree_test all.h.gch test
