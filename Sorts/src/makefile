ORTS_ROOT=../../..
SOURCES=$(shell ls *.cpp)
HEADERS=$(shell ls include/*.h)

OBJECTS=$(SOURCES:.cpp=.o) sorts_main.o
DEPS=$(SOURCES:.cpp=.d) sorts_main.d

sorts:
	cd $(ORTS_ROOT); make sorts

tags: $(SOURCES) $(HEADERS)
	ctags -R *.cpp *.C include/*.h

clean:
	cd $(ORTS_ROOT)/dep; rm -f $(DEPS)
	cd $(ORTS_ROOT)/obj/dbg; rm -f $(OBJECTS)
	cd $(ORTS_ROOT)/obj/opt; rm -f $(OBJECTS)


