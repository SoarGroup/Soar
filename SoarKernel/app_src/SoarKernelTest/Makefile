# Initial pass at a makefile for the Soar Kernel

# Automatically calculating the source files and including the auto 
# generated dependency files
SOURCES = $(notdir $(wildcard *.cpp))

CXXFLAGS = -w -g -DUNIX -DUSE_TCL -I/usr/local/include -I../../include/ -L../../bin -L../../bin -L. -L/usr/local/lib -L../../lib 

all: mainKernelTest

include $(SOURCES:.cpp=.d)

mainKernelTest: $(SOURCES:.cpp=.o)
	$(CXX) $(CXXFLAGS) $(SOURCES:.cpp=.o) -lsoarkernel -llua -llualib -o $@

clean:
	rm -f *.o *.d
	rm -f mainKernelTest

# This rule and the depscript file in the root directory create the .d
# dependencies files automatically
%.d: %.cpp
	$(SHELL) -ec '$(CXX) -MM $(CXXFLAGS) $< | sed -f ../../depscript - > $@'
