#include "MoveManager.H"
#include "Distance.H"

/********************************************************************
 * MoveManager::MoveManager
 ********************************************************************/
MoveManager::MoveManager(){
}

/********************************************************************
 * MoveManager::move_influence
 ********************************************************************/
void MoveManager::move_influence(sint4 x, sint4 y, InfluenceMap *influence,
		Squad *squadArg){
  assert(x >= 0);
  assert(y >= 0);
  assert(influence != NULL);
  assert(squadArg != NULL);
  
  // Variables
  squad = squadArg;
  update_speeds();
  update_goals(x, y);

  // Check that all units are moving
  FORS(i, squad->get_size()){
    GameObj* gob = squad->members[i];
    ScalarPoint goal = squad->goals[i];    

    // If our unit has stopped
    if(*gob->sod.speed == 0){
      squad->idleTime[i]++;
      
      // Traverse in one direction
      if(squad->idleTime[i] % 2 == 0){
	squad->dir[i]*= -1;
        traverse(*gob->sod.x, *gob->sod.y, goal, gob, i);
      }
      // Traverse in the other direction
      else 
        traverse(*gob->sod.x, *gob->sod.y, goal, gob, i);
    }
    // Otherwise, our unit is moving
    else{
      squad->idleTime[i] = 0;
      gob->set_action("move", move(force_vector(gob, i, influence), squad->speed[i]));
    }
  }
}

/********************************************************************
 * MoveManager::move_scout
 ********************************************************************/
void MoveManager::move_scout(InfluenceMap *influence, Squad *squadArg){
  assert(influence != NULL);
  assert(squadArg != NULL);

  // Variables
  sint4 width_pixels = influence->get_width_pixels();
  sint4 height_pixels = influence->get_height_pixels();
  squad = squadArg;



  FORS(i, squad->get_size()){
    GameObj *gob = squad->members[i];
    ScalarPoint goal;
   
    /*
     * I'm impressive.
     *
     *   * 1 * 2 *
     *   *       3
     *   *       *
     *   *       4
     *   * 6 * 5 *
     */
    switch(i){
    case 0:
      goal.x = width_pixels / 3; 
      goal.y = 0;
    break;
    case 1:
      goal.x = 2 * width_pixels / 3;
      goal.y = 0;
      break;
    case 2:
      goal.x = width_pixels;
      goal.y = height_pixels / 3;
      break;
    case 3:
      goal.x = width_pixels;
      goal.y = 2 * height_pixels / 3;
      break;
    case 4: 
      goal.x = 2 * width_pixels / 3;
      goal.y = height_pixels;
      break;
    case 5:
      goal.x = width_pixels / 3;
      goal.y = height_pixels;
      break;
    default:
      goal.x = width_pixels / 2;
      goal.y = width_pixels / 2;
      cout << " SHOUD NOT BE REACHED " << endl;
      assert(false);
      break;
    }

    // Assign goal
    squad->goals[i] = goal; 

    // If our unit has stopped
    if(*gob->sod.speed == 0){
      squad->idleTime[i]++;
      
      // Traverse in one direction
      if(squad->idleTime[i] % 2 == 0){
	squad->dir[i]*= -1;
        traverse(*gob->sod.x, *gob->sod.y, goal, gob, i);
      }
      // Traverse in the other direction
      else 
        traverse(*gob->sod.x, *gob->sod.y, goal, gob, i);
    }
    // Otherwise, our unit is moving
    else{
      squad->idleTime[i] = 0;
      gob->set_action("move", move(force_vector(gob, i, influence), squad->speed[i]));
    }
  }
}

/********************************************************************
 * MoveManager::move_direct
 ********************************************************************/
void MoveManager::move_direct(sint4 x, sint4 y, 
			      InfluenceMap *influence, Squad *squadArg){
  assert(influence != NULL);
  assert(squadArg != NULL);
  assert(x >= 0);
  assert(y >= 0);

  // Updates
  squad = squadArg;
  update_speeds();
  update_goals(x, y);

  // Move all units
  FORS(i, squad->get_size()){
    GameObj *gob = squad->members[i];
    gob->set_action("move", move(squad->goals[i], squad->speed[i]),
		    squad->speed[i]);
  }
}


/********************************************************************
 * MoveManager::force_vector
 ********************************************************************/
ScalarPoint MoveManager::force_vector(GameObj *gob, sint4 id, 
				     InfluenceMap *influence){
  pair<sint4, sint4> friendlyF;
  pair<double, double> targetF, rockF;
  ScalarPoint goal;
  sint4 xpos = *gob->sod.x;
  sint4 ypos = *gob->sod.y;
  rockF.first = rockF.second = 0.0;
  
  // set weighting values
  const sint4 targetWeight = 80;
  const sint4 rockWeight = 10;
  
  // calculate direction to target
  sint4 dx = squad->goals[id].x - xpos;
  sint4 dy = squad->goals[id].y - ypos;
  double length  = sqrt(dx * dx + dy * dy);
  targetF.first  = targetWeight * (dx / length);
  targetF.second = targetWeight * (dy / length);
  
  // calculate effect of terrain
  for(dx= -50; dx < 50; dx += 16){
    for(dy = -50; dy < 50; dy += 16){
      if((xpos + dx) > 0 && (ypos + dy) > 0 &&
         (xpos + dx) < influence->get_width_pixels() &&
	 (ypos + dy) < influence->get_height_pixels()){
	double influenceValue = influence->value_at(xpos+dx, ypos+dy);
	double length = sqrt(dx*dx + dy*dy);
   
	if(influenceValue > 1000.0){
          rockF.first  += -rockWeight* dx/length;
	  rockF.second += -rockWeight* dy/length;
        }
      }
    }
  }
  
  // calculate direction and speed of movement
  dx = (sint4) ((targetF.first  + rockF.first ));
  dy = (sint4) ((targetF.second + rockF.second));
  
  goal.x = xpos + dx;
  goal.y = ypos + dy;
    
  return goal;
}

/********************************************************************
 * MoveManager::traverse
 ********************************************************************/
void MoveManager::traverse(sint4 locx, sint4 locy, ScalarPoint goal,
			   GameObj *gob, sint4 id){
  ScalarPoint moveTo;
  sint4 dx = goal.x-locx;
  sint4 dy = goal.y-locy;
  
  if(dy == 0){
    moveTo.x = locx;
    moveTo.y = locy + (20 * squad->dir[id]);
    gob->set_action("move", move(moveTo, squad->speed[id]));
    return;
  }

  /* Here, we calculate the slope, then normalize our
   * movement vector, multiply the resulting components by
   * the number of steps we want to move, and add this to
   * our original location vector to determine our expected
   * position
   */
  double slope = -((double) dx) / ((double) dy);  
  double mag = sqrt(1+(slope*slope));

  moveTo.x = locx + (int) (20.0 / mag * squad->dir[id]);
  moveTo.y = locy + (int) (20.0 / mag * slope * squad->dir[id]);

  gob->set_action("move", move(moveTo, squad->speed[id]));
}

/********************************************************************
 * MoveManager::update_speeds 
 ********************************************************************/
void MoveManager::update_speeds(){
  ScalarPoint center;
  const real4 speedMult = 1.0;

  // Get center of moving units
  FORS(i, squad->get_size()){
    if (true){
      center.x += *squad->members[i]->sod.x;
      center.y += *squad->members[i]->sod.y;
    }
  }
  center.x /= squad->get_size();
  center.y /= squad->get_size();
  
  real4 meanDist = sqrt((squad->target.x - center.x) * 
			(squad->target.x - center.x) + 
			(squad->target.y - center.y) * 
			(squad->target.y - center.y));
  
  FORS(i, squad->get_size()){
    real4 curDist = sqrt((squad->target.x - *squad->members[i]->sod.x) * 
			 (squad->target.x - *squad->members[i]->sod.x) + 
			 (squad->target.y - *squad->members[i]->sod.y) * 
			 (squad->target.y - *squad->members[i]->sod.y));
    
    // If our current distance is less than 
    if (curDist < meanDist - 10)
      squad->speed[i] = (int)(*squad->members[i]->sod.max_speed * speedMult);
    else 
      squad->speed[i] = *squad->members[i]->sod.max_speed;
  }
}

/********************************************************************
 * MoveManager::update_goals
 ********************************************************************/
void MoveManager::update_goals(sint4 x, sint4 y){
  uint4 id = 0;
  for(sint4 i = -3; i <= 3; i++){
    for(sint4 j = -3; j <=3 && id < squad->members.size(); j++){
      sint4 unitDiameter = 2 * *squad->members[id]->sod.radius;
      ScalarPoint newGoal;
      newGoal.x = x + i * unitDiameter;
      newGoal.y = y + i * unitDiameter;
      squad->goals[id] = newGoal;
      id ++;
    }
  }
}

/********************************************************************
 * MoveManager::move
 ********************************************************************/
Vector<sint4> MoveManager::move(ScalarPoint loc, sint4 speed){
  Vector<sint4> params;
  params.push_back(loc.x);
  params.push_back(loc.y);
  params.push_back(speed);
  return params;
}

Vector<sint4> MoveManager::move(Scalar x, Scalar y, sint4 speed){
  ScalarPoint loc;
  loc.x = x;
  loc.y = y;
  return move(loc, speed);
}

/********************************************************************
 * MoveManager::sqrdist - used to determine if we are close
 * enough to a target.
 ********************************************************************/
sint4 MoveManager::sqrdist(sint4 x, sint4 y, sint4 x2, sint4 y2){
  return (x-x2)*(x-x2) + (y-y2)*(y-y2);
}

/********************************************************************
 * MoveManager::calc_goal
 ********************************************************************/
ScalarPoint MoveManager::calc_goal(){
  ScalarPoint center = squad->get_center();
  ScalarPoint goal;

  sint4 dx = squad->target.x - center.x;
  sint4 dy = squad->target.y - center.y;
  
  if(dy == 0 || dx == 0){
    goal.x = center.x;
    goal.y = center.y;
    return goal;
  }
  else {
    goal.x = center.x + dx/2;
    goal.y = center.y + dy/2;
    double slope = ((double) dy) / ((double) dx);
    
    double mag = sqrt(1+(slope*slope));
    goal.x = center.x + (int) (1 / mag);
    goal.y = center.y + (int) (1 / mag * slope);
    
    return goal;
  }
}

/********************************************************************
 * MoveManager::move_vector -- broken and not used
 ********************************************************************/
void MoveManager::move_vector(ScalarPoint goal, InfluenceMap *influence, GameObj *gob){
  assert(goal.x >= 0 && goal.y >= 0);
  assert(influence != NULL);
  assert(gob != NULL);

  Scalar x1 = goal.x - *gob->sod.x; // dx to goal
  Scalar y1 = goal.y - *gob->sod.y; // dy to goal
  Scalar h1 = (Scalar)sqrt(x1 * x1 + y1 * y1); // total distance to goal
  cout << "h1: " << h1 << endl;
  
  Scalar px = *gob->sod.x + (x1  * (*gob->sod.max_speed + *gob->sod.radius)) / h1;
  Scalar py = *gob->sod.y + (y1  * (*gob->sod.max_speed + *gob->sod.radius)) / h1;
  
  // Search for a walkable path

  double theta = asin((double)y1/h1);

  cout  << "theta (orig): " << theta << endl;
  
  while(influence->value_at(px, py) > influence->get_walkable()){
    // Add pi/2 to theta
    theta += M_PI_2;
    
    // Compute new goal position
    px = (Scalar)(*gob->sod.x + h1 * cos(theta));
    py = (Scalar)(*gob->sod.y + h1 * sin(theta));
    
    cout << "theta: " << theta << endl;
    cout << "px: " << px << endl;
    cout << "py: " << py << endl;
  }
  
  // Move the object
  gob->set_action("move", move(px, py, *gob->sod.max_speed));
}
