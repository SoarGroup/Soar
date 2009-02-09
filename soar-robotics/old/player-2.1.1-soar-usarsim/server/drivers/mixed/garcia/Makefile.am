AM_CPPFLAGS = -Wall -I$(top_srcdir) @GARCIA_CFLAGS@

noinst_LTLIBRARIES =
if INCLUDE_GARCIA
noinst_LTLIBRARIES += libgarcia.la
#EXTRA_LIBRARIES = libaIO.a libaUI.a libaStem.a libaTEAvm.a libaRobot.a
endif

libgarcia_la_SOURCES = garcia_mixed.cc garcia_mixed.h
#libgarcia_la_LIBADD = @GARCIA_LIBS@
#PLAYER_DRIVER_EXTRA_LIBS += @GARCIA_LIBS@