include /home/jzxu/CGAL-3.2/make/makefile_i686_Linux-2.6_g++-4.0.2

ORTS_ROOT=../../..
SOURCES= AttackFSM.cpp AttackManager.cpp AttackManagerRegistry.cpp AttackNearFSM.cpp AttributeSet.cpp BuildFSM.cpp DefendFSM.cpp FeatureMap.cpp FeatureMapManager.cpp FSM.cpp general.cpp GridMapTileGrouper.cpp IdleFSM.cpp MapManager.cpp MapRegion.cpp MineFSM.cpp MineManager.cpp MoveFSM.cpp OrtsInterface.cpp PerceptualGroup.cpp PerceptualGroupManager.cpp Rectangle.cpp SDLCanvas.cpp SoarAction.cpp SoarGameObject.cpp SoarInterface.cpp SortsCanvas.cpp Sorts.cpp SpatialDB.cpp

OBJECTS=$(SOURCES:.cpp=.o) sorts_main.o
DEPS=$(SOURCES:.cpp=.d) sorts_main.d

BLAH_LIB=/home/jzxu/CGAL-3.2/lib/i686_Linux-2.6_g++-4.0.2

sorts:
	cd $(ORTS_ROOT); make sorts

clean:
	cd $(ORTS_ROOT)/dep; rm -f $(DEPS)
	cd $(ORTS_ROOT)/obj/dbg; rm -f $(OBJECTS)
	cd $(ORTS_ROOT)/obj/opt; rm -f $(OBJECTS)
