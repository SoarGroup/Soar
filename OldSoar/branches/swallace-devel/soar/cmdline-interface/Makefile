

LIB_RUNTIME_DIR =	/usr/lib
TCL_LD_SEARCH_FLAGS =	

CC =			gcc
RANLIB =		ranlib

# Directories for installation
SRC_DIR =		.
TOP_DIR =		./..

# The location of the Soar kernel library, libsoarkernel.a

KERNEL_DIR =		../kernel
KERNEL_LIB_SPEC =	-L$(KERNEL_DIR) -lsoarkernel
KERNEL_INCLUDE_DIR =	../kernel



# Set these to be whatever flags you want to pass to your compiler.
# If the flags define ANSI C behavior, be sure to set them.
#

ANSI_FLAGS =		-fpcc-struct-return
SHLIB_CFLAGS =		-fPIC
DEFS =			-DSOAR_LIBRARY=\"$(SOAR_LIBRARY)\"  -DSTDC_HEADERS=1 -DHAVE_UNISTD_H=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_GETHOSTNAME=1 -DHAVE_GETTIMEOFDAY=1 -DHAVE_GETWD=1 
INCLUDE_FLAGS =		-I$(KERNEL_INCLUDE_DIR) 
CFLAGS =		-O -g -Wall $(ANSI_FLAGS) $(SHLIB_CFLAGS) $(DEFS) \
                        $(INCLUDE_FLAGS)

# The symbols below provide support for dynamic loading and shared
# libraries.  See configure.in for a description of what the
# symbols mean.  The values of the symbols are normally set by the
# configure script.  You shouldn't normally need to modify any of
# these definitions by hand.
MATH_LIBS =		-lieee -lm
LIBS =			$(KERNEL_LIB_SPEC)  -ldl \
                          $(MATH_LIBS) -lc




# The objects
OBJS =	parsing.o\
	soarInterfaceCommands.o\
	ask.o \
	demo_adder.o \
	demo_toh.o

all:	soar_app

lib:	libsoarnui.a

soar_app: $(OBJS) main.o 
	$(CC) $(CFLAGS) -o soar_app $(OBJS) main.o $(LIBS) 

tests: nit1.o $(OBJS)
	$(CC) $(CFLAGS) -o test1 nit1.o $(OBJS) $(LIBS)

libsoarnui.a: $(OBJS)
	$(AR) $(ARFLAGS) libsoarnui.a $(OBJS)
	$(RANLIB) libsoarnui.a

clean:
	rm -f *.o core *~ soar-app libsoarnui.a



