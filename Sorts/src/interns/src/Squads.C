#include "Squads.H"
#include "DrawOnTerrain2D.H"

/********************************************************************
 * Squad::Squad
 ********************************************************************/
Squad::Squad(){}

/********************************************************************
 * Squad::add
 ********************************************************************/
void Squad::add_unit(GameObj* unit){
  // Add each unit to the back of the squad
  members.push_back(unit);
  idleTime.push_back(0);
  speed.push_back(*unit->sod.max_speed);
  dir.push_back(1);

  ScalarPoint temp;
  temp.x = 0;
  temp.y = 0;
  goals.push_back(temp);

  return;
}

/********************************************************************
 * Squad::remove
 ********************************************************************/
void Squad::remove_unit(GameObj* unit){
  //kind of slow right now, might be able to improve
  //This iterates of the vector and removes a single
  //unit
  for(sint4 i = members.size() - 1; i >= 0; i--){
    if(members[i] == unit){
      members.erase(members.begin()+i);
      idleTime.erase(idleTime.begin()+i);
      goals.erase(goals.begin()+i);
      speed.erase(speed.begin()+i);
      dir.erase(dir.begin()+i);
    }
  }
}

/********************************************************************
 * Squad::get_member
 ********************************************************************/
GameObj *Squad::get_member(){
  if (get_size() > 0)
    return members[0];
  return (GameObj *)NULL;
}

/********************************************************************
 * Squad::get_members
 ********************************************************************/
Vector<GameObj *> Squad::get_members(){
  return members;
}

/********************************************************************
 * Squad::get_center
 ********************************************************************/
ScalarPoint Squad::get_center(){
  ScalarPoint center;
  center.x = 0;
  center.y = 0;

  for(sint4 i = 0; i < get_size(); i++){
    center.x += *members[i]->sod.x;
    center.y += *members[i]->sod.y;
  }

  center.x /= get_size();
  center.y /= get_size();

  return center;
}

/********************************************************************
 * Squad::get_idle_time
 ********************************************************************/
Vector<sint4> Squad::get_idle_time(){
  return idleTime;
}

/********************************************************************
 * Squad::size
 ********************************************************************/
sint4 Squad::get_size(){
  return members.size();
}









