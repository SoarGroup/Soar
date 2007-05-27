APP_DIR  := sorts
APP_LIBS := kernel network serverclient 
#SHARED_LIBS += -L/home/swinterm/soarsvn/SoarSuite/SoarLibrary/lib
SHARED_LIBS += -L/home/dsk07/rts/rts11s/soarsuite-8.6.3/SoarLibrary/lib
SHARED_LIBS += -lClientSML -lConnectionSML -lElementXML
INC_OPTS += -I./apps/sorts/src/include

APP := $(APP_DIR)
include config/app.rules
