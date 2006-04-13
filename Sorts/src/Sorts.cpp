
/* Sorts class

   highest-level class for the middleware
   has pointers to the various interfaces and managers

   most lower-level objects should have a pointer to this
*/

#include "include/Sorts.h"

Sorts::Sorts(SoarInterface* _si,
             OrtsInterface* _oi,
             GroupManager* _gm,
             MapManager* _mm,
             FeatureMapManager* _fmm) :
   SoarIO(_si),
   ORTSIO(_oi),
   gm(_gm),
   mm(_mm),
   fmm(_fmm) 
{

}

SoarInterface* Sorts::getSoarInterface() {
  return SoarIO;
}

OrtsInterface* Sorts::getOrtsInterface() {
  return ORTSIO;
}

GroupManager* Sorts::getGroupManager() {
  return gm;
}

MapManager* Sorts::getMapManager() {
  return mm;
}

FeatureMapManager* Sorts::getFeatureMapManager() {
  return fmm;
}

