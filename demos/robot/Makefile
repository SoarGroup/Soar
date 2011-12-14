SVS=../../src
FLAGS=-I$(SVS) -I$(SVS)/models -I/home/jzxu/local/include -L/home/jzxu/local/lib -g

all: splintersim splintersim2

splintersim: splintersim.cpp $(SVS)/linalg.cpp $(SVS)/linalg.h
	g++ $(FLAGS) -o splintersim splintersim.cpp $(SVS)/linalg.cpp

splintersim2: splintersim2.cpp $(SVS)/linalg.h
	g++ $(FLAGS) -o splintersim2 splintersim2.cpp

splintersim3: splintersim3.cpp $(SVS)/ipcsocket.h $(SVS)/ipcsocket.cpp $(SVS)/models/splinterenv.h
	g++ $(FLAGS) -o splintersim3 splintersim3.cpp $(SVS)/ipcsocket.cpp $(SVS)/common.cpp -lchipmunk
