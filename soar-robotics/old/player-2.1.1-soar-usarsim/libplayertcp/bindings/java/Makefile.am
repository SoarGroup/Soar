AM_CPPFLAGS = -g -Wall -I$(top_srcdir) @JNI_CFLAGS@

# files that must be built first
BUILT_SOURCES =
if BUILD_JPLAYER
BUILT_SOURCES += playertcp_java_wrap.cc
endif

EXTRA_DIST = playertcp_java.i playertcp_java_wrap.cc

lib_LTLIBRARIES =
if ENABLE_JPLAYER
lib_LTLIBRARIES += libplayertcp_java.la
endif

playertcp_java_wrap.cc: playertcp_java.i \
                        $(top_srcdir)/libplayertcp/playertcp.h
	rm -rf net
	mkdir -p net/sourceforge/playerstage/Jplayertcp
	swig -v -java -package net.sourceforge.playerstage.Jplayertcp -outdir net/sourceforge/playerstage/Jplayertcp -c++ -o $@ -I$(srcdir)/../../.. $<

libplayertcp_java_la_SOURCES = playertcp_java_wrap.cc
libplayertcp_java_la_LDFLAGS = $(top_builddir)/libplayertcp/libplayertcp.la $(top_builddir)/libplayerxdr/libplayerxdr.la $(top_builddir)/libplayercore/libplayercore.la $(top_builddir)/server/libplayerdrivers/libplayerdrivers.la $(top_builddir)/libplayercore/libplayererror.la $(PLAYER_VERSION_INFO) -rpath $(libdir)

javadir = $(prefix)/share/player/java/net/sourceforge/playerstage/Jplayertcp
java_DATA = 
if ENABLE_JPLAYER
java_DATA += $(srcdir)/net/sourceforge/playerstage/Jplayertcp/*.java
endif

dist-hook:
	mkdir -p $(distdir)/net/sourceforge/playerstage/Jplayertcp
	cp -r -p -v $(srcdir)/net/sourceforge/playerstage/Jplayertcp/* $(distdir)/net/sourceforge/playerstage/Jplayertcp

if DARWIN
if ENABLE_JPLAYER
install-exec-hook:
	cd $(DESTDIR)$(libdir) && ln -sf libplayertcp_java.dylib libplayertcp_java.jnilib 
endif
endif
