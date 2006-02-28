#include "AStarNode.h"
#include <list>
//Need to include the map to construct graph

class AStar{
 public:
	AStar();
	virtual ~AStar();

	void init(int, int, int, int);
	void findPath(std::list<AStarNode *> *path);
	void clearLists();


 private:
	Location start;
	Location goal;

  	std::list<AStarNode *> open_list;
  	std::list<AStarNode *> closed_list;
};
