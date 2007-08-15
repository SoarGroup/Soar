#ifndef MoveManager_H
#define MoveManager_H

#include "SimpleTerrain.H"
#include "TerrainModule.H"
#include "InfluenceMap.H"
#include "Squads.H"

using namespace std;

class MoveManager{

public:
  MoveManager();
  
  /**
   * Function: update_speeds();
   * -------------------------
   * Makes units faster or slower so that they will hopefully stay
   * together as a cohesive group
   */
  void update_speeds();
  
  /**
   * Function move_influence
   * -------------------------
   * Given the x and y coordinates for a target and an influence map,
   * this function moves all units to that location.
   */
  void move_influence(sint4 x, sint4 y, InfluenceMap* influence,
		      Squad *squad);

  /**
   * Function move_scout
   * -------------------------
   * Will move a squad to the corners of the map in order to scout.
   */
  void move_scout(InfluenceMap *influence, Squad *squadArg);

/**
   * Function move_direct
   * --------------------
   * Will directly move, no dodging
   */
  void move_direct(sint4 x, sint4 y, InfluenceMap *influence, Squad *squadArg);

/**
   * Function move_vector
   * --------------------
   * Given an x and y, will move around n stuff
   */
  void move_vector(ScalarPoint goal, InfluenceMap *influence, GameObj *gob);

  
private: 
  /**
   * Function: update_goals
   * ----------------------
   * Calculates where the game object aught to go to based on it's
   * target location.
   */
  void update_goals(sint4 x, sint4 y);
  /**
   * Function: force_vector
   * ---------------------
   * Calculates the direction and speed an object should move in
   * to reach it's goal
   */
  ScalarPoint force_vector(GameObj *gob, sint4 id, InfluenceMap *influence);
  
  /**
   * Function: move
   * --------------
   * Causes a single unit to move to a given location
   */
  Vector<sint4> move(ScalarPoint loc, sint4 speed);
  Vector<sint4> move(Scalar x, Scalar y, sint4 speed);
  
  /**
   * Function: traverse
   * ------------------
   * Forces units to move perpendicular to the direction of their
   * target location.
   */
  void traverse(sint4 x, sint4 y, ScalarPoint goal, GameObj* gob,
		sint4 id);
  
  /**
   * Function: calcGoal
   * ------------------
   * Calculates the best location for the squad to move to to reach its
   * goal.
   */
  ScalarPoint calc_goal();
  
  /**
   * Function: sqrdist()
   * -------------------
   * Used to determine if we are close enough to target.
   */
  sint4 sqrdist(sint4 x,sint4 y, sint4 x2, sint4 y2); 
 
protected: 
  Squad *squad;
};

#endif
