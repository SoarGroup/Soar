#ifndef Squads_H
#define Squads_H

#include "SimpleTerrain.H"
#include "TerrainModule.H"
#include "InfluenceMap.H"

using namespace std;


class Squad{
public:
  Squad();
  
  /**
   * Function: add
   * -------------
   * Takes a GameObj * as an argument, which should be a unit
   * (marine, worker, tank, etc.), and places it in the
   * vector of units in the squad.
   */
  void add_unit(GameObj *unit);
  
  /**
   * Function: remove
   * ---------------
   * Removes a dead unit from the squad.
   */
  void remove_unit(GameObj* unit);
  
  /**
   * Function: get_size
   * --------------
   * Returns the number of units within the squad.
   */
  sint4 get_size();

  /**
   * Function: get_center
   * --------------------
   * Returns a ScalarPoint of the center of the current squad
   */
  ScalarPoint get_center();
  
  /**
   * Function: get_member()
   * ---------------------
   * Returns the first member in the member vector of the squad
   */
  GameObj* get_member();
  
  /**
   * Function: get_members()
   * ----------------------
   * Returns the members vector. Should only be used to assign
   * actions to members;
   */
  Vector<GameObj *> get_members();
  
  /**
   * Function:: get_idle_time()
   * ------------------------
   * Returns the vector of idle time for each unit
   */
  Vector<sint4> get_idle_time();
  
  // Variables  
  Vector<GameObj *> members;
  Vector<sint4> idleTime;
  Vector<sint4> speed;
  Vector<sint4> dir;
  Vector<ScalarPoint> goals;

  ScalarPoint target;
  
};

#endif
