#
# 		Makefile for STI
#		~~~~~~~~~~~~~~~~
#
#	Douglas Pearson		Jan 2002
#
# This is a general purpose Makefile.  It contains a few powerful
# functions explained lower in the file: make srcs, make depend and
# make all.

# The name of the main executable

PROGRAM1 = simple1	# No shared library
PROGRAM2 = simple2	# Uses shared library
SHARED = libSoarToolJavaInterface1.so

# The key directories
#     BINDIR : where the executable goes
#     SRCDIR : where the source files are
#     OBJDIR : where the object files go
#
# Format is "/abc/def/" or "./sub/" or "../../sib/sib1/" etc.
# (Not just names e.g. "sub/", could cause problems in make depend)
# Names must end with "/".

BINDIR= ./
SRCDIR= ./
OBJDIR= ./

# Path to include files (e.g. -I/usr/include or -I../include)
# BADBAD: Not sure how to do this in a jdk version independent way
INCLUDES=-I/usr/java/jdk1.3.1_02/include -I/usr/java/jdk1.3.1_02/include/linux

# Libraries (e.g. -lm)
LIBS=libSTI1.so

# Name of the compiler to use (e.g. cc or gcc or g++)
# (Set compiler to CC, for the C++ compiler)
CC=g++

# Flags to use during compilation and linking
CFLAGS= -fPIC 
LDFLAGS=

#
# Sources, and Objects.
#

# Start of SRCS

SRCS =  ${SRCDIR}SoarToolJavaInterface.cpp

OBJS =  ${OBJDIR}SoarToolJavaInterface.o
 
# End of SRCS

# Generic compilation rule

.cpp.o:
	${CC} ${CFLAGS} ${INCLUDES} -o $@ -c $<

COMPILE = ${CC} ${CFLAGS} ${INCLUDES} -o $@ -c

#
#	make srcs
#	    This will scan the SCRDIR and collect all of the .c files.
#	    It will build a SRCS = and OBJS = macro for these files
#	    and add them to this Makefile.
#
#	    Uses "# Start of SRCS" and "# End of SRCS" to define where
#	    to put these macros, *deleting* everything between these
#	    comments in the process.
#	
#	    The old Makefile is stored in Makefile.bak1
#
#	make depend
#	    This generates a list of dependencies for each file in SRCS
#	    and adds a command to compile it, getting the source from
#	    the SRCDIR and putting the objects in OBJDIR.
#	    The dependencies are based on the includes used by
#	    each source program.  Currently I am stripping out any include
#	    files which have an absolute address.  This makes the Makefile
#	    less system dependent.  To change this make the next line
#	    ABSOLUTES = 1 (instead of ABSOLUTES = 0)

ABSOLUTES = 0

#
#	    Considerable effort is expended to ensure that these dependencies
#	    are generated relative to the macros SRCDIR and OBJDIR
#	    so these variables can be changed without regenerating the dependency list.
#
#	make all
#	    This is just a generic way to compile "PROGRAM".
#

# Just a generic way to compile the main program

all:	${SHARED}

# Build everything into the executable
${PROGRAM1} : ${OBJS}
	${CC} ${LDFLAGS} -o ${BINDIR}${PROGRAM1} ${OBJS} ${LIBS}

# Build everything into a shared library and then link that to the executable
${SHARED} : ${OBJS}
	${CC} -shared ${LDFLAGS} -o ${BINDIR}${SHARED} ${OBJS} ${LIBS}

${PROGRAM2} : ${SHARED}
	${CC} ${LDFLAGS} -o ${BINDIR}${PROGRAM2} ${SHARED} ${LIBS}

# Delete all of the object files -- BUGBUG: Need to make this use OBJDIR list
clean:
	rm -f ${OBJS}\
	${BINDIR}${SHARED}\
	${BINDIR}${PROGRAM1}\
	${BINDIR}${PROGRAM2}

# Make srcs will search the SRCDIR for all .c files and generate
# an entry of the form SRCS = ${SRCDIR}/file1.c ${SRCDIR}/file2.c ...
# Also an emtry of the form OBJS = ${OBJDIR}/file1.o ...
# It then inserts this entry into the Makefile between the Start of SRCS
# and End of SRCS lines.

srcs:
	( cd ${SRCDIR} ; \
	  ls *.c *.C *.cpp | \
	  awk ' BEGIN { rec = "\nSRCS = " } \
                { if (length(rec $$0) > 65) \
	             { print rec "\\" ; rec = "        $${SRCDIR}" $$0; } \
                  else rec = rec " $${SRCDIR}" $$0 } \
	        END { print rec "\n" } ' > /tmp/make.srcs )

	cat /tmp/make.srcs | \
	  sed -n 's/SRC/OBJ/gp' | \
	  sed -n -e 's/\.cpp/\.o/gp' -e 's/\.c/\.o/gp' -e 's/\.C/\.o/gp' > /tmp/make.objs
	echo " " >> /tmp/make.objs

	cat Makefile | sed -n '1,/^# Start of SRCS/p' > /tmp/make.before
	cat Makefile | sed -n '/^# End of SRCS/,$$p' > /tmp/make.after

	mv Makefile Makefile.bak1
	cat /tmp/make.before /tmp/make.srcs /tmp/make.objs \
            /tmp/make.after > Makefile
	rm /tmp/make.before /tmp/make.srcs /tmp/make.after /tmp/make.objs
	echo 'Done Sources & Objects'

# Make depend will scan each of the SRC files and determine which include
# files it depends on, thus building an effecient dependency list.
# Also adds necessary compilation rule, which is needed if we're compiling
# files in different directories.

# The process is quite complex.  Run cc -M to get the list of dependencies.
# Then use an awk script to compress these and add the OBJDIR field
# and the compile information.
# Then compute a sed pattern to replace the source directory with
# SRCDIR (which is tricky because must watch for "." and "/" both
# of which are special in regular expressions).  Then add it to
# the Makefile.

depend:
	${CC} -MM ${INCPATH} ${SRCS} | \
	awk 'BEGIN { start = 1 ; all = ${ABSOLUTES} ; } \
	     { if ($$1 != prev) \
                { print rec ; \
		  if (start != 1) \
		      { print "	$${COMPILE} " cfile "\n"; } \
		  rec = "$${OBJDIR}" $$1; \
		  start = 0 ; \
		  prev = $$1; \
		  cfile = $$2; } \
		{ if (length(rec $$2) > 55) \
		        { print rec ; rec = "$${OBJDIR}" $$1 ; \
			  if (substr($$2,1,1) != "/" || all == 1) rec = rec " " $$2 }\
		  else  { if (substr($$2,1,1) != "/" || all == 1) rec = rec " " $$2 }}}\
	     END { print rec ; \
		   print "	$${COMPILE} " cfile }'>/tmp/make.dep
	echo " " >> /tmp/make.dep

	echo 's?${SRCDIR}\/?$${SRCDIR}?g' | \
	sed -e 's?\.?\\.?' -e 's?\/?\\\/?' > /tmp/pattern
	cat /tmp/make.dep | sed -f /tmp/pattern > /tmp/make.depends

	cat Makefile | sed -n '1,/^# Start of Dependencies/p' > /tmp/make.before
	cat Makefile | sed -n '/^# End of Dependencies/,$$p' > /tmp/make.after

	mv Makefile Makefile.bak2
	cat /tmp/make.before /tmp/make.depends /tmp/make.after > Makefile
	rm /tmp/make.before /tmp/make.dep /tmp/make.after /tmp/pattern
	rm /tmp/make.depends
	echo "Done Dependencies"

# Start of Dependencies

${OBJDIR}test.o: ./test.C
	${COMPILE} ./test.C
 
# End of Dependencies

