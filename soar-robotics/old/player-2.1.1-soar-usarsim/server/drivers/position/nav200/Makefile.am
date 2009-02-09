noinst_LTLIBRARIES = 
if INCLUDE_SICKNAV200
noinst_LTLIBRARIES += libsicknav200.la
noinst_PROGRAMS = test_program_init test_program test
endif

AM_CPPFLAGS = -Wall -I$(top_srcdir)

libsicknav200_la_SOURCES = sicknav200.cc nav200.h nav200.cc

test_program_init_SOURCES = test_program_init.cpp
test_program_init_LDADD = libsicknav200.la 
test_program_SOURCES = test_program.cpp
test_program_LDADD = libsicknav200.la 
test_SOURCES = test.cpp
test_LDADD = libsicknav200.la 