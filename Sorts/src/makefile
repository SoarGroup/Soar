ORTS_ROOT=../../..
SOURCES= AttackFSM.cpp AttackManager.cpp AttackManagerRegistry.cpp AttackNearFSM.cpp AttributeSet.cpp BuildFSM.cpp DefendFSM.cpp FeatureMap.cpp FeatureMapManager.cpp FSM.cpp general.cpp GridMapTileGrouper.cpp IdleFSM.cpp MapManager.cpp MapRegion.cpp MineFSM.cpp MineManager.cpp MoveFSM.cpp OrtsInterface.cpp PerceptualGroup.cpp PerceptualGroupManager.cpp Rectangle.cpp SDLCanvas.cpp SoarAction.cpp SoarGameObject.cpp SoarInterface.cpp SortsCanvas.cpp Sorts.cpp SpatialDB.cpp

OBJECTS=$(SOURCES:.cpp=.o) sorts_main.o
DEPS=$(SOURCES:.cpp=.d) sorts_main.d

sorts:
	cd $(ORTS_ROOT); make sorts

clean:
	cd $(ORTS_ROOT)/dep; rm -f $(DEPS)
	cd $(ORTS_ROOT)/obj/dbg; rm -f $(OBJECTS)

