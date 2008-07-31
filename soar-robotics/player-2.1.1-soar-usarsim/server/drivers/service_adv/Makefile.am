noinst_LTLIBRARIES =
if INCLUDE_SERVICE_ADV_MDNS
noinst_LTLIBRARIES += libservice_adv_mdns.la
endif

AM_CPPFLAGS = -Wall -I$(top_srcdir) @HOWL_CFLAGS@
libservice_adv_mdns_la_SOURCES = mdns.cc
#libservice_adv_mdns_la_LDADD = @HOWL_LDFLAGS@
