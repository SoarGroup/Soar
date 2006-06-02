#include "BuildFSM.h"
#include "Rectangle.h"

Rectangle getBuildingBounds(BuildFSM::BUILDING_TYPE type, int centerX, int centerY) {
  int width, height;
  switch (type) {
    case BuildFSM::CONTROL_CENTER:
      width = 4 * 16 - 2;
      height = 4 * 16 - 2;
      break;
    case BuildFSM::BARRACKS:
      width = 4 * 16 - 2;
      height = 3 * 16 - 2;
      break;
    case BuildFSM::FACTORY:
      width = 4 * 16 - 2;
      height = 3 * 16 - 2;
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
  type = (BUILDING_TYPE) params[0];
  loc_x = params[1];
  loc_y = params[2];
  buildingBounds = getBuildingBounds(type, loc_x, loc_y);
  isMoving = false;
  building = false;
  finished = false;
}

int BuildFSM::update() {
  if (finished) {
    return FSM_SUCCESS;
  }
  if (building) {
    if (gob->get_int("is_mobile") == 1) {
      // is this a reliable test for completion?
      return FSM_SUCCESS;
    }
  }
  else {
    Circle unitBounds(*gob->sod.x, *gob->sod.y, *gob->sod.radius);
    if (buildingBounds.intersects(unitBounds)) {
      if (isMoving) {
        moveFSM->stop();
        isMoving = false;
      }

      // start building
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
    else if (!isMoving) {
      // begin moving toward build site
      if (moveFSM == NULL) {
        moveFSM = new MoveFSM(gob);
      }
      vector<sint4> moveParams;
      moveParams.push_back(loc_x);
      moveParams.push_back(loc_y);
      moveFSM->init(moveParams);
      moveFSM->update();
      isMoving = true;
    }
    else {
      if (moveFSM->update() == FSM_FAILURE) {
        // can't get to the place
        return FSM_FAILURE;
      }
    }
  }
  return FSM_RUNNING;
}
