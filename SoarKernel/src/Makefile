# Initial pass at a makefile for the Soar Kernel

# Automatically calculating the source files and including the auto 
# generated dependency files
SOURCES = $(notdir $(wildcard *.cpp))
include $(SOURCES:.cpp=.d)

CXXFLAGS = -g -fPIC -ansi -fpcc-struct-return -DUNIX -DUSE_TCL -D_DEBUG -I../include


libsoarkernel.a: $(SOURCES:.cpp=.o)
	ar r $@ $(SOURCES:.cpp=.o)
	ranlib libsoarkernel.a

clean:
	rm -f *.o *.d
	rm -f libsoarkernel.a

# This rule and the depscript file in the root directory create the .d
# dependencies files automatically
%.d: %.cpp
	$(SHELL) -ec '$(CXX) -MM $(CXXFLAGS) $< | sed -f ../depscript - > $@'
