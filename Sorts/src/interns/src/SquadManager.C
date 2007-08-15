#include "InternEventHandler.H"
#include "SquadManager.H"


using namespace std;
/********************************************************************
 * SquadManager::SquadManager
 ********************************************************************/
SquadManager::SquadManager(){
}

/********************************************************************
 * MoveManager::insert_units
 ********************************************************************/
void SquadManager::insertUnits(const Vector<ScriptObj::WPtr> &objects, sint4 owner){
  
  // Insert All Buildings
  FORALL(objects, it){
    GameObj* g = (*it)->get_GameObj();
    if(*(g->sod.owner) == owner && g->bp_name() == "controlCenter"){
      baseSquads[g] = new Squad;
    }
  }

  // Add units to building and type squads
  FORALL(objects, it){
    GameObj *g = (*it)->get_GameObj();
    string name = g->bp_name();
        
    //If a unit belongs to us and it is a marine, worker, or tank, put it
    //in the appropriate squad
    if((*(g->sod.owner) == owner)){
      if((name == "marine" || name == "worker" || name == "tank")){
	mapAdd(g);
	ourUnits.add_unit(g);
      } 
    }
    // Enemy Bases
    else if(name == "controlCenter"){
      enemyBases.add_unit(g);
    }
    // Marines Workers and Tanks
    else if(name == "marine" || name == "worker" || name == "tank")
      enemyUnits.add_unit(g);
  }
}

/********************************************************************
 * SquadManager::remove_units
 ********************************************************************/
void SquadManager::removeUnits(const Vector<ScriptObj::WPtr> &objects, sint4 owner){
  FORALL(objects, it){
    GameObj *g = (*it)->get_GameObj();
    string name = g->bp_name();
    cout << name << endl;
    //If a unit belongs to us and it is a marine, worker, or tank,
    //remove it from the appropriate squad
    
    // Our Units
    if(*(g->sod.owner) == owner){
      if(name == "marine" || name == "worker" || name == "tank"){
	
        //Remove the unit from it's base defined squad
	map<GameObj *, Squad *>::iterator itr = baseSquads.begin();
	while(itr != baseSquads.end()){
	  itr->second->remove_unit(g);
	  itr++;
	}
        ourUnits.remove_unit(g);
      }
    }
    // Control Center
    else if(name == "controlCenter"){
      enemyBases.remove_unit(g);
    }
    // Marine Worker Tank
    else if(name == "marine" || name == "worker" || name == "tank")
      enemyUnits.remove_unit(g);
  }
}

/********************************************************************
 * SquadManager::map_add
 ********************************************************************/
void SquadManager::mapAdd(GameObj* gob){
  if(baseSquads.size() > 0){
    
    // Default: unit belongs to the first building
    map<GameObj *, Squad *>::iterator itr = baseSquads.begin();
    GameObj *group = itr->first;
    sint4 minDist = (sint4) gob->distance_to(*(const Object *)itr->first);
    sint4 temp;
    
    // Determine the actual squad it belongs to 
    while(itr != baseSquads.end()){
      temp = (sint4) gob->distance_to(*(const Object *)itr->first);
      if(temp < minDist){
        group = itr->first;
        minDist = temp;
      }
      itr++;
    }

    // Add the unit to the appropriate squad
    baseSquads[group]->add_unit(gob);
  }
}

/********************************************************************
 * SquadManager::dist
 ********************************************************************/
sint4 SquadManager::dist(GameObj* gob1, GameObj* gob2){
  return (*gob2->sod.x - *gob1->sod.x) * (*gob2->sod.x - *gob1->sod.x) +
    (*gob2->sod.y - *gob1->sod.y) * (*gob2->sod.y - *gob1->sod.y);
}
  
/********************************************************************
 * SquadManager::destroy_bases
 ********************************************************************/
void SquadManager::destroy_bases(InfluenceMap *influence){
  ScalarPoint goal;
  real8 high;
  goal.x = goal.y = 0;
  high = -9999;

  // First move to highest influence enemy base
  if (enemyBases.get_size() > 0){
    FORS(i, enemyBases.get_size()){
      sint4 x, y;
      x = *enemyBases.members[i]->sod.x;
      y = *enemyBases.members[i]->sod.y;
      
      if (influence->value_at(x, y) > high){
	goal.x = x;
	goal.y = y;
	high = influence->value_at(x, y);
      }
    }
  }
  
  // Then move to highest influence enemy unit
  else if(enemyUnits.get_size() > 0){
    FORS(i, enemyUnits.get_size()){
      sint4 x, y;
      x = *enemyUnits.members[i]->sod.x;
      y = *enemyUnits.members[i]->sod.y;
      
      if (influence->value_at(x, y) > high){
	goal.x = x; 
	goal.y = y;
	high = influence->value_at(x, y);
      }
    }
  }
  
  
  if (baseSquads.begin() == baseSquads.end()){
    move.move_direct(goal.x, goal.y, influence, &ourUnits);
  }
  else{
    map<GameObj *, Squad *>::iterator itr = baseSquads.begin();
    while(itr != baseSquads.end()){
      if(itr->second->get_size() > 0)
	move.move_influence(goal.x, goal.y, influence, itr->second);
      itr++;
    }
  }  
}

/********************************************************************
 * SquadManager::worker_rush
 ********************************************************************/
void SquadManager::worker_rush(InfluenceMap *influence){
  ScalarPoint goal;
  real8  high = -9999;

  // If we have located the enemy base, move to it with all units
  if (enemyBases.get_size() > 0){
    FORS(i, enemyBases.get_size()){
      sint4 x, y;
      x = *enemyBases.members[i]->sod.x;
      y = *enemyBases.members[i]->sod.y;
      
      if (influence->value_at(x, y) > high){
	goal.x = x;
	goal.y = y;
	high = influence->value_at(x, y);
      }
    }
    
    if (baseSquads.begin() == baseSquads.end())
      move.move_direct(goal.x, goal.y, influence, &ourUnits);
    else {
      map<GameObj *, Squad *>::iterator itr = baseSquads.begin();
      while (itr != baseSquads.end()){
	if (itr->second->get_size() > 0)
	  move.move_direct(goal.x, goal.y, influence, itr->second);
	itr++;
      }
    }
    return;
  }
  
  // If there are no bases then scout for enemy bases
  move.move_scout(influence, &ourUnits);
}

/********************************************************************
 * SquadManager::gather_highest
 ********************************************************************/
void SquadManager::gather_highest(InfluenceMap *influence){
  ScalarPoint goal;
  goal.x = goal.y = 0;
  
  goal = influence->get_highest();
  
  if (baseSquads.begin() == baseSquads.end()){
    move.move_influence(goal.x, goal.y, influence, &ourUnits);
  }
  else{
    map<GameObj *, Squad *>::iterator itr = baseSquads.begin();
    while (itr != baseSquads.end()){
      if (itr->second->get_size() > 0)
	move.move_influence(goal.x, goal.y, influence, itr->second);
      itr ++;
    }
  }
}
  
  
  
  
  
