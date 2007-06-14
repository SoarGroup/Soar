CC=g++
LINK=g++
CFLAGS=-Wall -g
SOURCES= mapper.cpp matcher.cpp predicate.cpp reader.cpp rule.cpp 
HEADERS= matcher.h predicate.h reader.h rule.h
OBJECTS=$(SOURCES:.cpp=.o)
MAIN=mapper

CTAGS=ctags

$(MAIN): $(OBJECTS)
	$(LINK) $(OBJECTS) -o $@

.cpp.o: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(MAIN)

tags: $(SOURCES) $(HEADERS)
	$(CTAGS) $(SOURCES) $(HEADERS)
