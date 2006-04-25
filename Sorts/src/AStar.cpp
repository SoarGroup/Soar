#include "include/AStar.h"


AStar::AStar()
{

}

AStar::~AStar()
{
 clearLists();
}


void AStar::init(int x1, int y1, int x2, int y2)
{
 start.x = x1;
 start.y = y1;

 goal.x = x2;
 goal.y = y2;
}


void AStar::findPath(std::list<AStarNode *> *path)
{

/*
  //Create and insert start node into open list
  	//Create and insert new nodes as they are needed (might be slow - Keep nodes live?)

  //Once goal node is found, trace parent pointers and push tiles into path list.
	//Alter MoveFSM so that it is consistent with this definition 
  //return

*/

}




//This might be very slow
void AStar::clearLists()
{
 AStarNode *tmp;

 while(!open_list.empty())
 {
  tmp = open_list.front();
  open_list.pop_front();
  delete tmp;
 }

 while(!closed_list.empty())
 {
  tmp = closed_list.front();
  closed_list.pop_front();
  delete tmp;
 }

}
