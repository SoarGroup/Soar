#include"include/AStarNode.h"

AStarNode::AStarNode()
{
 g = 0;
 h = 0;
 parent = 0;
}

AStarNode::~AStarNode()
{
 parent = 0;
}

void AStarNode::setCoordinates(int x, int y)
{
 loc.x = x;
 loc.y = y;
}


void AStarNode::setParent(AStarNode *p)
{
 parent = p;
}

AStarNode *AStarNode::getParent()
{
 return parent;
}

void AStarNode::setMoveCost(int c)
{
 g = c*c;
}


void AStarNode::setGoalNode(int x, int y)
{
 h = (loc.x-x)*(loc.x-x) + (loc.y-y)*(loc.y-y);
}

int AStarNode::getNodeValue()
{
 return g + h;
}
