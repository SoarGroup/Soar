#include "BuildFSM.h"
#include "Rectangle.h"


#define msg cout << "BUILDFSM: "

Rectangle getBuildingBounds(BuildingType type, int centerX, int centerY) {
  int width, height;
  switch (type) {
    case CONTROL_CENTER:
      width = 4 * 16 - 2;
      height = 4 * 16 - 2;
      break;
    case BARRACKS:
      width = 4 * 16 - 2;
      height = 3 * 16 - 2;
      break;
    case FACTORY:
      width = 4 * 16 - 2;
      height = 3 * 16 - 2;
      break;
    default:
      assert(false);
      break;
  }
  Rectangle r(centerX, centerY, width, height, true);
  return r;
}

BuildFSM::BuildFSM(GameObj* _gob) 
: FSM(_gob)
{ 
  name = OA_BUILD;  
}

BuildFSM::~BuildFSM() {
  if (moveFSM != NULL) {
    delete moveFSM;
  }
}

void BuildFSM::init(vector<sint4> params) {
  FSM::init(params);
  type = (BuildingType) params[0];
  loc_x = params[1];
  loc_y = params[2];
  buildingBounds = getBuildingBounds(type, loc_x, loc_y);
  startedMove = false;
  building = false;
  startedBuild = false;
}

int BuildFSM::update() {
  
  int moveStatus;
  
  if (building) {
    if (gob->get_int("is_mobile") == 1) {
      // is this a reliable test for completion?
      msg << "building completed.\n";
      return FSM_SUCCESS;
    }
  }
  else {
    Circle unitBounds(*gob->sod.x, *gob->sod.y, *gob->sod.radius);
    if (not startedBuild and buildingBounds.intersects(unitBounds)) {
      msg << "reached bounds while moving\n";
      if (startedMove) {
        moveFSM->stop();
      }
      startedBuild = true;
    }
    else if (!startedMove) {
      // begin moving toward build site
      if (moveFSM == NULL) {
        moveFSM = new MoveFSM(gob);
      }
      vector<sint4> moveParams;
      moveParams.push_back(loc_x);
      moveParams.push_back(loc_y);
      msg << "moving to location.\n";
      moveFSM->init(moveParams);
      moveFSM->update();
      startedMove = true;
    }
    else if (startedBuild) {
      // start building
      msg << "starting build.\n";
      Vector<sint4> buildParams;
      buildParams.push_back(loc_x);
      buildParams.push_back(loc_y);
      switch (type) {
        case CONTROL_CENTER:
          gob->set_action("build_controlCenter", buildParams);
          break;
        case BARRACKS:
          gob->set_action("build_barracks", buildParams);
          break;
        case FACTORY:
          gob->set_action("build_factory", buildParams);
          break;
      }
      building = true;
    }
    else {
      moveStatus = moveFSM->update();
      if (moveStatus == FSM_FAILURE) {
        // can't get to the place
        msg << "move has failed!\n";
        return FSM_FAILURE;
      }
      else if (moveStatus == FSM_SUCCESS) {
        if (not buildingBounds.intersects(unitBounds)) {
          msg << "no intersection:\n";
          msg << "unit: " <<  unitBounds.x << "," << unitBounds.y 
              << " rad " << unitBounds.r << endl;
          msg << "bldg x: " 
              << buildingBounds.xmin << "->" << buildingBounds.xmax 
              << " y " << buildingBounds.ymin << "->" << buildingBounds.ymax  << endl;
        }
        else {
          msg << "ERROR: why wasn't this caught above?\n";
        }
        startedBuild = true;
      }
      else if (moveStatus != FSM_RUNNING) {
        // unreachable, probably
        msg << "fail, move returned " << moveStatus << endl;
        return FSM_FAILURE;
      }
      else {
        msg << "movefsm running\n";
      }
    }
  }
  if (building) {
    msg << "building in progress\n";
  }
  
  return FSM_RUNNING;
}
