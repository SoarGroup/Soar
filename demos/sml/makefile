LIBPATH=-L/home/jzxu/SoarSuite/out/lib -L/home/jzxu/local/lib
INCPATH=-I/home/jzxu/SoarSuite/out/include
FLAGS=-g
LIBS=-lClientSML -lConnectionSML -lElementXML -larmadillo -lBulletCollision -lLinearMath -ldl -lpthread -lm

sml: sml.cpp
	g++ $(LIBPATH) $(INCPATH) $(FLAGS) -o sml sml.cpp $(LIBS)

