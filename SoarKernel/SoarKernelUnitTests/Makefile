# A simple makefile for the unit tests

SOURCES = $(notdir $(wildcard *.cpp))

all: unitTest

.PHONY: all

include $(SOURCES:.cpp=.d)

CXXFLAGS = -g -fPIC -ansi -fpcc-struct-return -DUNIX -DUSE_TCL -D_DEBUG -I../include -I../cppunit-install/include -L../lib -L../cppunit-install/lib

unitTest: $(SOURCES:.cpp=.o)
	$(CXX) $(CXXFLAGS) $(SOURCES:.cpp=.o) -lsoarkernel -lcppunit -o $@

clean:
	rm -f *.o *.d
	rm -f unitTest

# This rule and the depscript file in the root directory create the .d
# dependencies files automatically
%.d: %.cpp
	$(SHELL) -ec '$(CXX) -MM $(CXXFLAGS) $< | sed -f ../depscript - > $@'
