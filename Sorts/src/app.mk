APP_DIR  := sorts
APP_LIBS := kernel network serverclient

SHARED_LIBS += -lSoarKernelSML -lElementXML

APP := $(APP_DIR)
include config/app.rules
