CC=g++
LINK=g++
CFLAGS=-Wall -ggdb3
SOURCES= mapper.cpp matcher.cpp predicate.cpp rule.cpp 
HEADERS= matcher.h mapper.h predicate.h reader.h rule.h
OBJECTS=$(SOURCES:.cpp=.o)
MAIN=mapper

CTAGS=ctags

$(MAIN): $(OBJECTS) $(HEADERS)
	$(LINK) $(OBJECTS) -o $@

.cpp.o: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(MAIN)

tags: $(SOURCES) $(HEADERS)
	$(CTAGS) $(SOURCES) $(HEADERS)
