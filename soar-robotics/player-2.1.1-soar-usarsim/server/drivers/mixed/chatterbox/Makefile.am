noinst_LTLIBRARIES =
if INCLUDE_CHATTERBOX
noinst_LTLIBRARIES += libchatterbox.la
endif

ROBOSTIX=/home/gumstix/vR1078experimental/robostix

# NASTY HACK! RTV
AM_CPPFLAGS = -Wall -I$(top_srcdir) -I$(ROBOSTIX)/Shared -I$(ROBOSTIX)/gumstix/Common -I$(ROBOSTIX)/gumstix/roomba

#libchatterbox_la_SOURCES = cb_driver.cc cb_i2c.c cb_i2c.h
libchatterbox_la_SOURCES = cb_driver.cc cb_i2c.c

# this stuff is the right way to do it, but it's not respected by libplayerdrivers
# so instead the missing library is inserted in libplayerdrivers's Makefile.am
#libchatterbox_la_LIBADD = -lroboi2c
#libchatterbox_la_LDFLAGS = -L$(ROBOSTIX)/gumstix/roomba