#ifndef SPathFinder_H
#define SPathFinder_H

#include "Sorts.h"
#include "GameChanges.H"
#include "PathEvent.H"
#include "SEnvironment.H"
#include "PathProcessor.H"
#include "EventHandler.H"


class SPathFinder: public EventHandler 
{
 public:
	SPathFinder(const Sorts *);
	~SPathFinder();
	
	static const sint4 FIND_PATH_MSG;
	static const sint4 FIND_PATH_STOP;

	bool handleEvent(const Event *e);
	
	PathProcessor pp;

 private:
	void initEnv();

	void findPath(const PathEvent *e);
	void updatePaths(Point2i *a, Point2i *b);
	

	GameStateModule *gsm;
	
	int width;
	int height;
	int tile_points;

	std::queue<PathEvent> taskQ;
	SEnvironment *env;
	SimpleMap<SizeCell> *map;
};

#endif
