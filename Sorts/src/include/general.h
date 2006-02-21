#ifndef general_h
#define general_h

#include<list>
#include<utility>
#include<iostream>

#include "sml_Client.h"

using namespace std;

// I had to move this here because of forward declaration crazyness
typedef list<pair<string, int> > groupPropertyList;
struct groupPropertyStruct {
  list<pair<string, int> > stringIntPairs;
  list<pair<string, string> > stringStringPairs;
  list<int> regionsOccupied;
};

string catStrInt(const char* str, int x);

string int2str(int x);

const char* getCommandParameter(sml::Identifier* c, const char *name);

double squaredDistance(double x1, double y1, double x2, double y2);

#endif
