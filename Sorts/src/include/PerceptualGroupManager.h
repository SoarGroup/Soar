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
#ifndef PerceptualGroupManager_h
#define PerceptualGroupManager_h

#include "SoarGameObject.h"
#include "PerceptualGroup.h"
#include "SoarInterface.h"

class Sorts;


struct ltGroupPtr {
  bool operator()(PerceptualGroup* g1, PerceptualGroup* g2) const {
    double d1 = g1->getDistToFocus();
    double d2 = g2->getDistToFocus();
    if (d1 == d2) {
      // give an arbitrary order if distance is the same
      return ((unsigned int)g1 < (unsigned int)g2);
    }
    return (d1 < d2);
  }
};

class PerceptualGroupManager {
  public:
    PerceptualGroupManager();
    ~PerceptualGroupManager();

    void initialize();

    void updateGroups();
    bool assignActions();
    void processVisionCommands();

    void updateQueryDistances(); 

    void makeNewGroup(SoarGameObject* object);
    // used by ORTSInterface when it sees a new object- create a group for it
    
  private:
    VisionParameterStruct visionParams;
    /* in general.h:
    struct VisionParameterStruct {
      int centerX;
      int centerY;
      int viewWidth;
      int focusX;
      int focusY;
      bool ownerGrouping;
      int numObjects;
      int groupingRadius;
    };*/
    
    void prepareForReGroup();
    void reGroup();
    void generateGroupData();
    void adjustAttention(bool rebuildFeatureMaps);

    void removeGroup(PerceptualGroup*);
    void remakeGroupSet();

    // perceptual groups are input to Soar, directly and in feature maps
    // Soar can control how they are created, adjusting the grouping radius
    // and allowing grouping by owner or not.

    // this set is maintained in sorted order, items toward the front
    // are closer to the center of focus, and have priority to go on the
    // input link.
    set <PerceptualGroup*, ltGroupPtr> perceptualGroups;
    
    set <pair<string, int> > staleGroupCategories;
    

    void setAllCategoriesStale();

    int counter;

};

struct perceptualGroupingStruct {
  SoarGameObject* object;
  PerceptualGroup* group;
  bool assigned;
  bool oldGroup;
  int x,y;
};

#endif // PerceptualGroupManager_h
