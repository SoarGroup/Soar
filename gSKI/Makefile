###########################################################################
# NOTE: The following variables may need to be modified based on
# your system configuration
###########################################################################

###########################################################################
#
# End of section that may need modification
#
###########################################################################

# Root directory for the gSKI package
ROOTDIR = .

# Application source directories
TESTAPPSOURCEDIR = $(ROOTDIR)/app_src/gSKITestApp

# Library source directories
SOURCEDIR = $(ROOTDIR)/src
DOCDIR = $(ROOTDIR)/Documentation

# Soar library directory (the location of libsoarkernel.a)
SOARLIBROOTDIR = $(ROOTDIR)/lib_src/SKx
SOARLIBDIR = $(SOARLIBROOTDIR)/lib

# Soar header file location
SOARHEADERDIR = $(ROOTDIR)/lib_src/SKx/include
BOOSTHEADERDIR = $(ROOTDIR)/boost

# External lib location
EXTLIBDIR = $(ROOTDIR)/bin

# This should be soarkernel for gcc < 3 and soarkernel3 for gcc >= 3.
SOAR_KERNEL = soarkernel

# MegaUnit test header location
MUTHEADERDIR = $(ROOTDIR)/MUT/exports

# Setting the VPATH variable to better handle the source distribution
VPATH = $(TESTAPPSOURCEDIR):\
	$(SOURCEDIR)

# Setting some target directories
APPTARGETDIR = $(ROOTDIR)/bin
LIBTARGETDIR = $(ROOTDIR)/lib
GSKIHEADERDIR = $(ROOTDIR)/include

# Automatically considering all cpp files in the various source directories
# to be source files for the appropriate application or library
TESTAPPSOURCES = $(notdir $(wildcard $(TESTAPPSOURCEDIR)/*.cpp))

LIBSOURCES = $(notdir $(wildcard $(SOURCEDIR)/*.cpp))

GSKIHEADERS = $(notdir $(wildcard $(GSKIHEADERDIR)/*.h))

# The -Wno-ctor-dtor-privacy disables an annoying error warning about
# only having private destructors.
CXXFLAGS = -I$(GSKIHEADERDIR) -I$(SOARHEADERDIR) -I$(MUTHEADERDIR) -I$(SOURCEDIR) -L$(EXTLIBDIR) -DUNIX -DNO_MEGA_UNIT_TESTS -Wall -Wno-ctor-dtor-privacy -g

# Default target is the test harness which should build everything else
# (This has to come before the included files below due to the default
#  goal selection rules of make.)
default: all

# Makes everything
all: soarkernel testapp gskilib_shared gskilib_static TAGS

# Including all the dependency files for the various source files
include $(TESTAPPSOURCES:.cpp=.d) $(LIBSOURCES:.cpp=.d)

# Targets that don't directly correspond to filenames
.PHONY: clean distclean doc soarkernel

# RDF commented out the building of the cwrapper and tclwrapper until the 
# interface is more stable
#cwrapper_shared cwrapper_static tclwrapper_shared tclwrapper_static


# Simple target for making tags 
TAGS: $(TESTAPPSOURCES) $(LIBSOURCES)
	ctags -eR --languages=-Java

# Removes all intermediate files created during the build process
clean:
	rm -f *.o
	rm -f *.d
	rm -f TAGS tags
	$(MAKE) -e -C $(SOARLIBROOTDIR) clean
	rm -f $(APPTARGETDIR)/gSKITestApp
	rm -Rf $(DOCDIR)/html
	rm -f $(APPTARGETDIR)/libgSKI.so
	rm -f $(LIBTARGETDIR)/libgSKI.so
	rm -f $(LIBTARGETDIR)/libgSKI.a

#
soarkernel:
	$(MAKE) -e -C $(SOARLIBROOTDIR) libsoar

# The top level rules for the various applications. These typically depend
# on the executable located in the app. target directory.
testapp: $(APPTARGETDIR)/gSKITestApp

# The top level rules for the various libraries. These typically depend on the
# shared library located in the library target directory.
gskilib_shared: soarkernel $(LIBTARGETDIR)/libgSKI.so

# For making the documentation
docs:
	cd $(DOCDIR)/..; bin/doxygen

# TODO: Add a build target that builds a staticly linked  version of the gSKITestApp

# These rules handle the actual creation of the applications in the app target directory
# (Responsible only for the final linking step, the automatically generated dependencies
#  handle the creation of the object files from the source files.)
$(APPTARGETDIR)/gSKITestApp: $(TESTAPPSOURCES:.cpp=.o) $(LIBTARGETDIR)/libgSKI.so
	$(CXX) $(CXXFLAGS) $(TESTAPPSOURCES:.cpp=.o) -L$(APPTARGETDIR) -lgSKI -o $@

# These rules handle the actual creation of the libraries in the lib target directory
$(LIBTARGETDIR)/libgSKI.so: $(LIBSOURCES:.cpp=.o) $(SOARLIBDIR)/libsoarkernel.a
	$(CXX) -pipe -shared $(CXXFLAGS) -L$(SOARLIBDIR) $^ -l$(SOAR_KERNEL) -lpcreposix -lpcre -o $@
	rm -f $(APPTARGETDIR)/libgSKI.so
	ln -fs ../$(LIBTARGETDIR)/libgSKI.so $(APPTARGETDIR)/libgSKI.so

# For automatically generating dependencies for C/C++ files
#
# These rules are responsible for bootstrapping the automatic dependency calculation
# .d files. These .d files are then included as separate make files above. See the
# README 
%.d: %.cpp
	$(SHELL) -ec '$(CXX) -MM $(CXXFLAGS) $< | sed -f depscript - > $@'

%.d: %.c
	$(SHELL) -ec '$(CC) -MM $(CXXFLAGS) $< | sed -f depscript - > $@'




