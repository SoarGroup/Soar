APP_DIR  := interns
APP_LIBS := kernel network serverclient gfxclient pathfinding/path ai/low pathfinding/simple_terrain

APP_EXT_HD   +=
APP_EXT_LIBS :=

APP := $(APP_DIR)
include config/app.rules
