noinst_LTLIBRARIES =
if INCLUDE_MRICP
noinst_LTLIBRARIES += libmricp.la
endif

AM_CPPFLAGS = -Wall -I$(top_srcdir) -I$(top_srcdir)/server/drivers/mixed/mricp/include @MRICP_CFLAGS@

libmricp_la_SOURCES = src/geometry2D.cpp src/icp.cpp src/lasermodel.cpp src/map.cpp \
  src/mricp_driver.cpp src/nn.cpp src/Timer.cpp \
  include/geometry2D.h \
  include/icp.h \
  include/lasermodel.h \
  include/map.h \
  include/nn.h \
  include/Timer.h
