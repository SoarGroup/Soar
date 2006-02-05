APP_DIR  := sorts
APP_LIBS := kernel network serverclient

SHARED_LIBS += -lSoarKernelSML -lElementXML
INC_OPTS += -I/opt/ORTS/orts_jan_09_06/orts3/apps/sorts/src/include

APP := $(APP_DIR)
include config/app.rules
