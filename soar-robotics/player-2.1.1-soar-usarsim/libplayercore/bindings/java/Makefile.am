AM_CPPFLAGS = -g -Wall -I$(top_srcdir) @JNI_CFLAGS@

# files that must be built first
BUILT_SOURCES =
if BUILD_JPLAYER
BUILT_SOURCES += playercore_java_wrap.cc playercore_java_wrap.h playercore_casts.i playercore_arraysofclasses.i
endif

EXTRA_DIST = playercore_java.i parse.py playercore_java_wrap.cc playercore_java_wrap.h playercore_casts.i playercore_arraysofclasses.i

lib_LTLIBRARIES =
if ENABLE_JPLAYER
lib_LTLIBRARIES += libplayercore_java.la
endif

playercore_arraysofclasses.i: playercore_casts.i

playercore_casts.i: $(top_srcdir)/libplayercore/player.h parse.py
	rm -rf net
	python $(srcdir)/parse.py $(srcdir)/../../player.h playercore_casts.i playercore_arraysofclasses.i net/sourceforge/playerstage/Jplayercore net/sourceforge/playerstage/Jplayercore player.java


playercore_java_wrap.h: playercore_java_wrap.cc
playercore_java_wrap.cc: playercore_java.i playercore_casts.i \
                         playercore_arraysofclasses.i\
                         $(srcdir)/../../configfile.h \
                         $(srcdir)/../../device.h \
                         $(srcdir)/../../devicetable.h \
                         $(srcdir)/../../driver.h \
                         $(srcdir)/../../drivertable.h \
                         $(srcdir)/../../error.h \
                         $(srcdir)/../../globals.h \
                         $(srcdir)/../../message.h \
                         $(srcdir)/../../player.h \
                         $(srcdir)/../../playercommon.h \
                         $(srcdir)/../../playertime.h \
                         $(srcdir)/../../addr_util.h
	swig -v -java -package net.sourceforge.playerstage.Jplayercore -outdir net/sourceforge/playerstage/Jplayercore -c++ -o $@ -I$(srcdir)/../../.. $<

#all-local:
#	javac *.java

libplayercore_java_la_SOURCES = playercore_java_wrap.cc playercore_java_wrap.h
libplayercore_java_la_LDFLAGS = $(top_builddir)/libplayercore/libplayererror.la $(top_builddir)/libplayercore/libplayercore.la $(top_builddir)/server/libplayerdrivers/libplayerdrivers.la $(PLAYER_VERSION_INFO) @PLAYER_DRIVER_EXTRA_LIBS@ -rpath $(libdir)

javadir = $(prefix)/share/player/java/net/sourceforge/playerstage/Jplayercore
java_DATA = 
if ENABLE_JPLAYER
java_DATA += $(srcdir)/net/sourceforge/playerstage/Jplayercore/*.java
endif

dist-hook:
	mkdir -p $(distdir)/net/sourceforge/playerstage/Jplayercore
	cp -r -p -v $(srcdir)/net/sourceforge/playerstage/Jplayercore/* $(distdir)/net/sourceforge/playerstage/Jplayercore

if DARWIN
if ENABLE_JPLAYER
install-exec-hook:
	cd $(DESTDIR)$(libdir) && ln -sf libplayercore_java.dylib libplayercore_java.jnilib 
endif
endif
