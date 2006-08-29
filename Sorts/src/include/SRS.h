#ifndef SRS_H
#define SRS_H

#include "SRSShape.h"
#include "CGALSupport.h"
#include <string>
#include <set>
#include <list>
using namespace std;

enum twoObjectQueryType {
  RCC_DR,
  RCC_PO,
  RCC_EQ,
  RCC_PP,
  RCC_PPI,
  ORIENTATION,
  ALLOCENTRIC_ORIENTATION,
  DISTANCE,
  SHORTEST_DIST_CLEAR,
  BIGGER
};

class SpatialReasoningSystem {
  public:
    SpatialReasoningSystem();
    void setWorldBounds(list<pair <double, double> > worldBounds);
    void initCanvas(double, double, double);
    void insertCircle(double x, double y, double radius, double i, double j, 
                      double speed, int id, string name);
    void insertPolygon(list<pair<double, double> >& points, double i, double j, 
                       double speed, int id, string name);
    void removeShape(int id);

    double twoObjectQuery(twoObjectQueryType type, int referenceId, int primaryId);
    void objectProjectionQuery(int objID, double time, double& x, double& y);
    void printAllRelativeOrientations();  
    void boxIntersectCallback(const CGALBox* a, const CGALBox* b);
  private:
    void drawPolygon(CGALPolygon*, int, int, int);
    bool worldIsPolygon;
    CGALPolygon worldPolygon;
    map<int, SRSShape*> shapes;
    SDLCanvas canvas;
    map<CGALBox*, SRSShape*> boxToShape;
    vector<CGALBox*> boxes;
    bool shortestDistanceClearQuery(SRSShape*, SRSShape*);
    bool polygonIntersect(CGALPolygon* poly);
    CGALPolygon* lastIntersectPolygon;
    bool lastIntersect;
};

string queryToString(twoObjectQueryType t);
void boxIntersectCallbackWrapper(const CGALBox* a, const CGALBox* b);


#endif
