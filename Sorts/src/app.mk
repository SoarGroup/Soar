APP_DIR  := sorts
APP_LIBS := kernel network serverclient gfxclient path ai/low

//SHARED_LIBS += -lSoarKernelSML -lElementXML
INC_OPTS += -I/opt/ORTS/orts_jan_09_06/orts3/apps/sorts/src/include

APP := $(APP_DIR)
include config/app.rules
