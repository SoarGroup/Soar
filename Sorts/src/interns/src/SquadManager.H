#ifndef SquadManager_H
#define SquadManager_H

#include "Squads.H"
#include "Vector.H"
#include "GameChanges.H"
#include <map>
#include "InfluenceMap.H"
#include "MoveManager.H"

class SquadManager{
  
public:
  /**
   * Constructor: SquadManager
   * -------------------------
   * Constructs a SquadManager instance to manage
   * our groups of squads with.
   */
  SquadManager();
  
  /**
   * Function: insertUnits
   * ---------------------
   * Given a vector with some number of new units, this
   * function adds these units to their appropriate squads.
   * If no appropriate squad exists, this function will
   * create a new squad.
   */
  void insertUnits(const Vector<ScriptObj::WPtr>& objects, sint4 owner);
  
  /**
   * Function: removeUnits
   * ---------------------
   *  Given a vector with some number of dead units, this function
   *  removes these units from their appropirate squads.
   */ 
  void removeUnits(const Vector<ScriptObj::WPtr>& objects, sint4 owner);

  /**
   * Function: mapAdd
   * ----------------
   * Add's a unit to one of the squads in the map of squads.
   * This map links a squad to the building these group of units
   * started out next to
   */
  void mapAdd(GameObj *gob);
  
  /**
   * Function: dist
   * --------------
   * Returns the distance between the midpoints of two game
   * objects
   */
  sint4 dist(GameObj* gob1, GameObj* gob2);
  
  /**
   * Function: influencedMovement
     * ----------------------------
     * Orders all squads to move their units based on
     * the influence map
     */

  void worker_rush(InfluenceMap *influence);
  void destroy_bases(InfluenceMap *influence);
  void gather_highest(InfluenceMap *influence);
  
private:
  map<GameObj *, Squad *> baseSquads;
  Vector<Squad *> squadManager;
  Squad ourUnits;
  Squad enemyBases;
  Squad enemyUnits;
  sint4 owner;
  
  MoveManager move;
};

#endif
