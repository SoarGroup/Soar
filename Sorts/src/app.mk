APP_DIR  := sorts
APP_LIBS := kernel network serverclient newpath

SHARED_LIBS += -L/home/swinterm/cgal/CGAL-3.2.1/lib/i686_Linux-2.6_g++-4.0.2 
SHARED_LIBS += -lClientSML -lConnectionSML -lElementXML -lCGAL -lgmp -lmpfr
INC_OPTS += -I./apps/sorts/src/include 
INC_OPTS += -I/home/swinterm/cgal/CGAL-3.2.1/include 
INC_OPTS += -I/home/swinterm/cgal/CGAL-3.2.1/include/CGAL/config/i686_Linux-2.6_g++-4.0.2/

APP := $(APP_DIR)
include config/app.rules
