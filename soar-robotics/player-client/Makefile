PREFIX=/home/voigtjr/sandbox
SOAR_HOME=/home/voigtjr/soar/SoarSuite

FLAGS=-Wall -g -Werror

INCLUDES=-I$(PREFIX)/include/player-2.1 -I$(SOAR_HOME)/Core/ClientSML/include -I$(SOAR_HOME)/Core/ElementXML/include -I$(SOAR_HOME)/Core/ConnectionSML/include
LIBINCLUDES=-L$(PREFIX)/lib
LIBS=-lplayerc++ -lplayerc -lm -lplayerxdr -lplayererror -lClientSML -lConnectionSML -lElementXML

all: soarplayer

RunThread.o: RunThread.cpp RunThread.inl RunThread.h
	g++ $(FLAGS) -c -o RunThread.o $(INCLUDES) RunThread.cpp
	
Console.o: Console.cpp Console.inl Console.h
	g++ $(FLAGS) -c -o Console.o $(INCLUDES) Console.cpp
	
SoarPlayerClient.o: SoarPlayerClient.cpp SoarPlayerClient.inl SoarPlayerClient.h
	g++ $(FLAGS) -c -o SoarPlayerClient.o $(INCLUDES) SoarPlayerClient.cpp
	
soarplayer.o: soarplayer.cpp Console.h Console.inl SoarPlayerClient.h SoarPlayerClient.inl
	g++ $(FLAGS) -c -o soarplayer.o $(INCLUDES) soarplayer.cpp

soarplayer: soarplayer.o SoarPlayerClient.o Console.o RunThread.o
	g++ $(FLAGS) -o soarplayer Console.o SoarPlayerClient.o soarplayer.o RunThread.o $(LIBINCLUDES) $(LIBS)
	
clean:
	rm -f *.o soarplayer
