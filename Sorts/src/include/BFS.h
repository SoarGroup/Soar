/*
    This file is part of Sorts, an interface between Soar and ORTS.
    (c) 2006 James Irizarry, Sam Wintermute, and Joseph Xu

    Sorts is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Sorts is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Sorts; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA    
*/
#ifndef BFS_H_
#define BFS_H_

#include <set>
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
  BFS(Point rootLoc, int _xmax, int _ymax);
  ~BFS();
 
  bool insertEdge(Point p1, Point p2, list<Point>* cycle);
  void takeOver(BFS* other, Point myPoint, Point otherPoint);

  int size() { return id2Node.size(); }

private: // functions
  void createNode(BFSNode* par, Point loc);
  void findCycle(BFSNode* n1, BFSNode* n2, list<Point>& cycle);
  void findBoundaryCycle(BFSNode* n1, BFSNode* n2, list<Point>& cycle);

private:
  int idCounter;
  bool combined;
  map<int, BFSNode*>   id2Node;
  map<Point, BFSNode*> loc2Node;

  set<BFSNode*> boundaryLeaves;

  int xmax;
  int ymax;
};

#endif
