APP_DIR  := sorts
APP_LIBS := kernel network serverclient gfxclient path ai/low newpath

SHARED_LIBS += -lClientSML -lConnectionSML -lElementXML
INC_OPTS += -I./apps/sorts/src/include 

APP := $(APP_DIR)
include config/app.rules
