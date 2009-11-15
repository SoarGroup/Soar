SOAR_INCLUDE_PATH=-I$(SOAR_HOME)/Core/ClientSML/include -I$(SOAR_HOME)/Core/ConnectionSML/include -I$(SOAR_HOME)/Core/ElementXML/include
SOAR_LIB_PATH=-L$(SOAR_HOME)/SoarLibrary/lib
SOAR_LIBS=-lClientSML -lConnectionSML -lElementXML

CC=gcc
CPPC=g++
INCLUDE_PATH=-Iinc $(SOAR_INCLUDE_PATH)
LIB_PATH=$(SOAR_LIB_PATH)
LIBS=$(SOAR_LIBS)
CPPFLAGS=-g $(INCLUDE_PATH)

EXECS=testsoarquery epmem
HEADERS=$(wildcard inc/*.h)
SRCS=$(filter-out $(addprefix src/,$(addsuffix .cpp,$(EXECS))),$(wildcard src/*.cpp))

OBJS=$(addprefix obj/, $(notdir $(SRCS:.cpp=.o)))
SQLITE_OBJS=obj/sqlite3.o
	
all: $(EXECS)

testsoarquery: $(OBJS) $(SQLITE_OBJS) src/testsoarquery.cpp
	$(CPPC) $(CPPFLAGS) $(LIB_PATH) -o $@ $^ $(LIBS)
	
epmem: $(OBJS) $(SQLITE_OBJS) src/epmem.cpp
	$(CPPC) $(CPPFLAGS) $(LIB_PATH) -o $@ $^ $(LIBS)

obj/sqlite3.o: src/sqlite3.c inc/sqlite3.h
	$(CC) $(CPPFLAGS) -c $< -o $@

$(OBJS): obj/%.o: src/%.cpp $(HEADERS)
	$(CPPC) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf obj/*
	rm -f $(EXECS)