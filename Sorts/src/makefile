ORTS_ROOT=../../..
SOURCES=AStar.cpp AStarNode.cpp FSM.cpp general.cpp GridMapTileGrouper.cpp GroupManager.cpp IdleFSM.cpp MapManager.cpp MapRegion.cpp MineFSM.cpp MoveFSM.cpp OrtsInterface.cpp Rectangle.cpp SoarGameGroup.cpp SoarGameObject.cpp SoarInterface.cpp FeatureMapManager.cpp FeatureMap.cpp SoarAction.cpp
OBJECTS=$(SOURCES:.cpp=.o) sorts_main.o
DEPS=$(SOURCES:.cpp=.d) sorts_main.d

sorts:
	cd $(ORTS_ROOT); make sorts

clean:
	cd $(ORTS_ROOT)/dep; rm -f $(DEPS)
	cd $(ORTS_ROOT)/obj/dbg; rm -f $(OBJECTS)
