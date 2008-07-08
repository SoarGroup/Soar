PREFIX=/home/voigtjr/sandbox
SOAR_HOME=/home/voigtjr/soar/SoarSuite

INCLUDES=-I$(PREFIX)/include/player-2.1 -I$(SOAR_HOME)/Core/ClientSML/include -I$(SOAR_HOME)/Core/ElementXML/include -I$(SOAR_HOME)/Core/ConnectionSML/include
LIBINCLUDES=-L$(PREFIX)/lib
LIBS=-lplayerc++ -lplayerc -lm -lplayerxdr -lplayererror -lClientSML -lConnectionSML -lElementXML

all: soarplayer

RunThread.o: RunThread.cpp RunThread.inl RunThread.h
	g++ -Wall -g -c -o RunThread.o $(INCLUDES) RunThread.cpp
	
Console.o: Console.cpp Console.inl Console.h
	g++ -Wall -g -c -o Console.o $(INCLUDES) Console.cpp
	
SoarPlayerClient.o: SoarPlayerClient.cpp SoarPlayerClient.inl SoarPlayerClient.h
	g++ -Wall -g -c -o SoarPlayerClient.o $(INCLUDES) SoarPlayerClient.cpp
	
soarplayer.o: soarplayer.cpp
	g++ -Wall -g -c -o soarplayer.o $(INCLUDES) soarplayer.cpp

soarplayer: soarplayer.o SoarPlayerClient.o Console.o RunThread.o
	g++ -Wall -g -o soarplayer Console.o SoarPlayerClient.o soarplayer.o RunThread.o $(LIBINCLUDES) $(LIBS)
	
clean:
	rm -f *.o soarplayer
