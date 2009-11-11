SOAR_HOME=../soar/nlderbin-epmem-smem2
SOAR_INCLUDE_PATH=-I$(SOAR_HOME)/Core/ClientSML/include -I$(SOAR_HOME)/Core/ConnectionSML/include -I$(SOAR_HOME)/Core/ElementXML/include
SOAR_LIB_PATH=-L$(SOAR_HOME)/SoarLibrary/lib
SOAR_LIBS=-lClientSML -lConnectionSML -lElementXML

CC=gcc
CPPC=g++
INCLUDE_PATH=-Iinc $(SOAR_INCLUDE_PATH)
LIB_PATH=$(SOAR_LIB_PATH)
LIBS=$(SOAR_LIBS)
CPPFLAGS=-g $(INCLUDE_PATH)

HEADERS=inc/*

SRCS=FloatSymbol.cpp IdentifierSymbol.cpp IntegerSymbol.cpp SimpleEpmem.cpp SoarQuery.cpp StringSymbol.cpp Symbol.cpp SymbolFactory.cpp WME.cpp SoarEpmem.cpp SimpleSymbolFactory.cpp

OBJS=$(SRCS:.cpp=.o)

store: $(OBJS) src/store.cpp
	$(CPPC) $(CPPFLAGS) $(LIB_PATH) -o store $(OBJS) src/store.cpp $(LIBS)

sqlite3.o: src/sqlite3.c inc/sqlite3.h
	$(CC) $(CPPFLAGS) -c $?

$(OBJS): %.o: src/%.cpp $(HEADERS)
	$(CPPC) $(CPPFLAGS) -c $< -o $@