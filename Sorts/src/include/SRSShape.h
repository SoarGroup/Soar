#ifndef SRSShape_H
#define SRSShape_H

#include "CGALSupport.h"
#include "SDLCanvas.h"
#include <string>
using namespace std;

class SRSShape {
  public:
    // create a circle at x,y with orientation i,j
    SRSShape(double x, double y, double radius, double i, double j,
             double, string, SDLCanvas*);
    // create a polygon with the list of points and orientation i,j
    SRSShape(list<pair<double, double> >& points, double i, double j,  
             double, string, SDLCanvas*);

    void deleteShape();
    void draw();

    int getRelativeOrientationOf(SRSShape*);
    int getAllocentricRelativeOrientationOf(SRSShape*);
    bool RCC_DR(SRSShape*);
    bool RCC_PO(SRSShape*);
    bool RCC_EQ(SRSShape*);
    bool RCC_PP(SRSShape*);
    bool RCC_PPi(SRSShape*);
    double getDistanceTo(SRSShape*);
    double getDistanceTo(double x, double y);
    bool isBiggerThan(SRSShape*);
    bool isAdjacentTo(SRSShape*);
    bool isBetween(SRSShape*, SRSShape*);
    CGALSegment translateCentroid(double deltaT);

    CGALDirection getOrientation() { return orientation; }
    CGALPoint getCentroid() { return centroid; }
    CGALCircle* getCircle() { return circle; }
    CGALPolygon* getPolygon() { return polygon; }
    bool getIsCircle() { return isCircle; }

  private:
    string name;
    bool isCircle;
    CGALPolygon* polygon;
    CGALCircle* circle;
    CGALDirection orientation;
    CGALPoint centroid;
    double speed;
    SDLCanvas* canvas;
    
    list<CGALRay> acceptanceAreaRays;
    list<CGALRay> allocentricAcceptanceAreaRays;

    list<SDLCanvasShape*> canvasObjs;
};

string SRS_catStrInt(const char* str, int x);

#endif
