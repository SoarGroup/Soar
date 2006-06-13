#ifndef BFS_H_
#define BFS_H_

#include <map>
#include <list>
#include <algorithm>

#include "Point.h"

using namespace std;

struct BFSNode {
  int id;
  BFSNode* par;
  Point loc;
  list<BFSNode*> children; // necessary for rotations (when combining trees)

  BFSNode(int _id, BFSNode* _par, Point _loc)
  : id(_id), par(_par), loc(_loc)
  { }

  void rotate(BFSNode* newPar) {
    BFSNode* oldPar = par;
    par = newPar;

    if (oldPar != NULL) {
      children.push_back(oldPar);
      list<BFSNode*>::iterator childPos =
        find(oldPar->children.begin(), oldPar->children.end(), this);
      assert(childPos != oldPar->children.end());
      oldPar->children.erase(childPos);
      // call on old parent to maintain tree consistency
      oldPar->rotate(this);
    }
  }
};

class BFS {
public:
  BFS(Point rootLoc);
  ~BFS();
 
  bool insertEdge(Point p1, Point p2, list<Point>* cycle);
  void takeOver(BFS* other, Point myPoint, Point otherPoint);

  int size() { return id2Node.size(); }

private: // functions
  void createNode(BFSNode* par, Point loc);
  void findCycle(BFSNode* n1, BFSNode* n2, list<Point>& cycle);

private:
  int idCounter;
  bool combined;
  map<int, BFSNode*>   id2Node;
  map<Point, BFSNode*> loc2Node;
};

#endif
