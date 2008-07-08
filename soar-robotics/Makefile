PREFIX=/home/voigtjr/sandbox
SOAR_HOME=/home/voigtjr/soar/SoarSuite

soarplayer: soarplayer.cpp
	g++ -o soarplayer -I$(PREFIX)/include/player-2.1 soarplayer.cpp -I$(SOAR_HOME)/Core/ClientSML/include -I$(SOAR_HOME)/Core/ElementXML/include -I$(SOAR_HOME)/Core/ConnectionSML/include -L$(PREFIX)/lib -lplayerc++ -lplayerc -lm -lplayerxdr -lplayererror -lClientSML -lConnectionSML -lElementXML
