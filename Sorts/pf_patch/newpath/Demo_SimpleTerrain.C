// $Id: Demo_SimpleTerrain.C,v 1.9 2006/06/09 18:46:14 ddeutscher Exp $

// This is an ORTS file 
// (c) Michael Buro, Sami Wagiaalla, David Deutscher
// licensed under the GPL

#include "Demo_SimpleTerrain.H"
#include "Demo_ST_PFEngine.H"
#include "Global.H"
#include "Options.H"

using namespace std;

#define TEST(x)
//#define TEST(x) x

namespace Demo_SimpleTerrain {
  // Sorts additions
  void ST_Terrain::findPath(const Object* gob, const Loc &l2, Path &path) {
    pfEngine->find_path(gob, l2, path);
  }
  void ST_Terrain::findPath(const Loc &l1, const Loc &l2, Path &path) {
    pfEngine->find_path(l1, l2, path);
  }
  void ST_Terrain::insertImaginaryWorker(Loc l) {
    pfEngine->insertImaginaryWorker(l);
  }
  void ST_Terrain::removeImaginaryWorker(Loc l) {
    pfEngine->removeImaginaryWorker(l);
  }
  void ST_Terrain::removeDynamicObjs() {
    pfEngine->removeDynamicObjs();
  }
  void ST_Terrain::insertDynamicObjs() {
    pfEngine->insertDynamicObjs();
  }
  void ST_Terrain::removeControlCenters() {
    pfEngine->removeControlCenters();
  }
  void ST_Terrain::insertControlCenters() {
    pfEngine->insertControlCenters();
  }
void ST_Terrain::findPath(const Object* gob, const Object* l2, Path &path) {
    sint4 x1, y1; l2->get_center(x1, y1);
    Loc l;
    l.x = x1;
    l.y = y1;
    sint4 x2, y2; gob->get_center(x2, y2);
    Loc lg;
    lg.x = x2;
    lg.y = y2;
    pfEngine->remove_object(l2);
    pfEngine->clearGobLocation(l2);
    pfEngine->remove_object(gob);
    pfEngine->clearGobLocation(gob);
    pfEngine->find_path(lg, l, path);
    pfEngine->insert_object(l2);
    pfEngine->insert_object(gob);
  }


  //-----------------------------------------------------------------------------
  void ST_Terrain::add_options()
  {
    ::Options o("SimpleTerrain");
    o.put("-gran",  sint4(4),  "granularity of the fine grid default 4x4");
    o.put("-costonly", "represent boundaries only by higher edge cost");
  }

  //-----------------------------------------------------------------------------
  void ST_Terrain::init(sint4 tiles_x_, sint4 tiles_y_, sint4 tile_points_, sint4 me, sint4 neutral)
  {
    TerrainBasicImp<ST_Task>::init(tiles_x_, tiles_y_, tile_points_, me, neutral);
    sint4 gran;
    Options::get("-gran", gran);
    pfEngine = boost::shared_ptr<PFEngine>(new PFEngine(tiles_x_, tiles_y_, tile_points_, gran));
  }

  //-----------------------------------------------------------------------------
  void ST_Terrain::add_obj(const Object *obj)
  {
    TerrainBasicImp<ST_Task>::add_obj(obj);
    assert(pfEngine!=NULL);
    pfEngine->insert_object(obj);
  }

  //-----------------------------------------------------------------------------
  void ST_Terrain::update_obj(const Object *obj)
  {
    TerrainBasicImp<ST_Task>::update_obj(obj);
    assert(pfEngine!=NULL);
    pfEngine->update_object(obj);
  }

  //-----------------------------------------------------------------------------
  void ST_Terrain::remove_obj(const Object *obj)
  {
    // shorthand
    typedef ObjIsGoalInTaskId::iterator OGIT;

    // remove from world representation
    assert(pfEngine!=NULL);
    pfEngine->remove_object(obj);

    // If we intended to stop it, we don't need to anymore
    objsToStop.erase(obj);

    // If this is the target of any existing task...
    pair<OGIT,OGIT> range = objIsGoalInTaskId.equal_range(obj);
    for( OGIT i = range.first ; i != range.second ; i++ ) {
      // Change the target of the task to the last known location of the object
      FIND(taskId2task, ti2tIt, i->second);
      assert(ti2tIt != taskId2task.end());
      ti2tIt->second->task.goal.target = Goal::LOCATION;
      ti2tIt->second->task.goal.obj->get_center(ti2tIt->second->task.goal.loc.x, 
                                                ti2tIt->second->task.goal.loc.y);
    }

    TerrainBasicImp<ST_Task>::remove_obj(obj);
  }

  //-----------------------------------------------------------------------------
  void ST_Terrain::add_segments(const Vector<Segment> &segs)
  {
    TerrainBasicImp<ST_Task>::add_segments(segs);
    assert(pfEngine!=NULL);
    FORALL(segs,i)
      pfEngine->insert_boundary(*i);
    invalidate_paths(segs);
  }

  //----------------------------------------------------------------------------------
  void ST_Terrain::cancel_task(const Object *obj)
  {
    objsToStop.insert(obj);
    TerrainBasicImp<ST_Task>::cancel_task(obj);
  }

  //----------------------------------------------------------------------------------
  void ST_Terrain::remove_task(TaskId tid)
  {
    FIND (taskId2task, it, tid);
    if( it != taskId2task.end() ) {
      Task& task = it->second->task;
      objsToStop.insert(task.objs.begin(), task.objs.end());
    }
    TerrainBasicImp<ST_Task>::remove_task(tid);
  }


  //----------------------------------------------------------------------------------
  real8 ST_Terrain::find_path(const Loc &/*l1*/, const Loc &/*l2*/, sint4 /*radius*/, Path * /*path*/, ConsiderObjects /*consider*/)
  {
    assert(0); // unimplemented yet
    return -1;
  }

  real8 ST_Terrain::find_path(const Object * /*obj*/, const Goal &/*goal*/, Path * /*path*/, ConsiderObjects /*consider*/)
  {
    assert(0); // unimplemented yet
    return -1;
  }

  //----------------------------------------------------------------------------------

  // fixme: we only check the path of the *center* of the unit vs the new boundaries,
  // and we should account for its size (radius).
  void ST_Terrain::invalidate_paths(const Vector<Segment> &segments)
  {
    FORALL(tasks, ti) {
      if( !ti->isValid() ) continue;
      // fixme: better handling of zcat != ON_LAND ?
      if( ti->path->obj->get_zcat() != Object::ON_LAND ) continue;

      FORALL(segments, si) {
        if( ti->path->is_intersect(*si) ) {
          // this path is invalid, mark as pending for (re)planning
          pendingTasks.push_back(ti);
          ti->isPending = true;
          cout << "Marked an invalid path" << endl;
          break;
        }
      }
    }
  }

  //----------------------------------------------------------------------------------
  void ST_Terrain::execute_tasks(Vector<TerrainBase::MoveCmd> &cmds, Vector<TerrainBase::StatusMsg> &msgs)
  {
    // Execute all active tasks
    for (Tasks::iterator j = tasks.begin(); j != tasks.end();) 
    {
      Tasks::iterator i = j++;

      // Report failure of planning (no path at all)
      if( i->noPathFound ) {
        TerrainBase::StatusMsg s;
        // fixme: this is not robust to future changes in the meaning of TaskId.
        s.task_id = (TerrainBase::TaskId)&(*i);
        s.obj = *(i->task.objs.begin());
        s.type = TerrainBase::StatusMsg::NO_PATH_FAILURE;
        msgs.push_back(s);
        continue;
      }

      // Skip invalid tasks.
      if( !i->isValid() ) { 
        TEST(cout << *i->task.objs.begin() << " task not valid" << endl;)
        continue;
      }

      UnitPath &cur_path = *(i->path);
      const Object *obj = cur_path.obj;

      // Skip tasks that are waiting for a retry
      if (cur_path.sleeping()) {
        TEST(cout << obj << " path sleeping" << endl;)
        continue;
      }

      // Skip objects for which we gave a (move) action that is still pending
      if (obj->is_pending_action()) {
        TEST(cout << obj << " pending action" << endl;)
        continue;
      }

      bool atGoal = is_at_goal(obj, i->task);

      // Look ahead: if the object will get to a waypoint in the next tick,
      // give the next action now the prevent it from wasting a tick on stopping.
      // This will cut a single tick at each waypoint, though it will really prevent 
      // the object from stopping only if there's no (network or processing) lag
      // between this client and the server.
      // atWaypoint can be true only if the current waypoint is an intermediate and
      // not the *final* waypoint/goal.
      bool atWaypoint = cur_path.current_target > 0 &&  // reverse order, goal==0
                        is_at_location(obj, cur_path.path.at(cur_path.current_target));
      
      // Skip a moving object which is currently not at a waypoint/goal
      if( (obj->get_speed() > 0 || obj->get_moving() != 0) && !atWaypoint && !atGoal ) {
        TEST(cout << obj << " atWaypoint=" << atWaypoint << " speed=" << obj->get_speed() << " is_moving=" << obj->get_moving() << endl;)
        continue;
      }

      // ok, so it needs attention:
      // 1. maybe it had reached the goal
      // 2. maybe it's at a waypoint (with the lookahead)
      // 3. maybe it needs the first orders
      // 4. maybe it unexpectedly collided with something

      if( atGoal ) {

        TEST(cout << obj << " arrived" << endl;)

        TerrainBase::StatusMsg s;
        // fixme: this is not robust to future changes in the meaning of TaskId.
        s.task_id = (TerrainBase::TaskId)&(*i);
        s.obj = obj; 
        s.type = TerrainBase::StatusMsg::ARRIVED;
        msgs.push_back(s);

      } else if( atWaypoint || cur_path.no_orders_yet ) {
        TEST(cout << obj << " atWaypoint=" << atWaypoint << " cur_path.no_orders_yet=" << cur_path.no_orders_yet << endl;)

        // if we are at a waypoint, move to next waypoint
        if( atWaypoint )
          cur_path.current_target--;
        cur_path.no_orders_yet = false;
        add_move_command(cur_path, cmds);

      } else {

        // Unexpected stop. The current policy is:
        // 1-2. First time => wait a little and send the move command again.
        // 3-4. Second stop at the same place => mark as failed and replan.
        // 5. Third stop at the same place => report failure.

        TEST(cout << obj << " unexpectedly stopped, collision_step=" << cur_path.collision_step << endl);

        Loc cur_position; obj->get_center(cur_position.x, cur_position.y);

        // First collision in this position:
        if( cur_path.spot != cur_position ) {
          cur_path.collision_step = 1;
          cur_path.mark_spot(cur_position);
          // sleep for a short random time, 500-999 millisecs
          cur_path.sleep(500 + (rand.rand_uint4() % 500));
        } else {
          cur_path.collision_step++;

          // Just finished the sleeping:
          if( cur_path.collision_step == 2 ) {
            add_move_command(cur_path, cmds);
          }
          // Second collision
          else if( cur_path.collision_step == 3 ) {
            i->isFailed = true;
          }
          // Back from replanning
          else if( cur_path.collision_step == 4 ) {
            add_move_command(cur_path, cmds);
          }
          // Third collision
          else if( cur_path.collision_step == 5 ) {
            TerrainBase::StatusMsg s;
            // fixme: this is not robust to future changes in the meaning of TaskId.
            s.task_id = (TerrainBase::TaskId)&(*i);
            s.obj = obj;
            s.type = TerrainBase::StatusMsg::MOVEMENT_FAILURE;
            msgs.push_back(s);
          }
        }
      }
    }
    
    // Stop objects (whose task was cancelled etc), and that did not yet 
    // get a new move command
    FORALL(cmds, c) {
      objsToStop.erase(c->obj);
    }
    FORALL(objsToStop, oi) {

      const Object *obj = *oi;
/*    // Not good, since our view might not be fully updated: --David
      if (obj->get_moving() == 0) {
        continue;
      }*/

      TerrainBase::MoveCmd m;
      m.obj = obj;
      obj->get_center(m.next_loc.x, m.next_loc.y); // doesn't really matter
      m.speed = 0; // 0 -> stop
      cmds.push_back(m);
    }
    objsToStop.clear();

    // remove finished tasks
    FORALL (msgs, t) {
      clean_task(t->task_id);
    }
  }

  void ST_Terrain::add_move_command(const UnitPath& path, Vector<TerrainBase::MoveCmd> &cmds)
  {
    TerrainBase::MoveCmd m;
    m.obj = path.obj;
    m.next_loc = path.path.at(path.current_target);
    m.speed = path.obj->get_max_speed();
    cmds.push_back(m);
    TEST(cout << m.obj << " sending move command, target=(" << m.next_loc.x << "," << m.next_loc.y << ")" << endl;)
  }

  //----------------------------------------------------------------------------------
  bool ST_Terrain::plan_tasks(uint4 /*max_time*/)
  {
    // plan a failed path
    bool did_something = plan_failed_task();

    // plan a pending (new) task
    if (!did_something)
      did_something = plan_pending_task();

    return did_something;
  }

  //----------------------------------------------------------------------------------
  bool ST_Terrain::plan_pending_task(void)
  {
    if (pendingTasks.empty()) return false;

    TIter tit = pendingTasks.front(); pendingTasks.pop_front();
    // mark the task as handled
    tit->isPending = false;

    // object to move
    assert(tit->task.objs.size() > 0);
    const Object *obj = *(tit->task.objs.begin());

    // if the object is moving, stop it first and come back to it later
    if (obj->get_speed() > 0 || obj->get_moving()) {
      objsToStop.insert(obj);
      pendingTasks.push_back(tit);
      return false;
      // fixme: might be better to try another pending task already in this tick...
    }

    // target location
    Loc goal;
    if (tit->task.goal.target == Goal::LOCATION)
      goal = tit->task.goal.loc;
    else {
      assert (tit->task.goal.target == Goal::OBJ);
      tit->task.goal.obj->get_center(goal.x, goal.y);
    }

    // plan
    Path newpath;
    pfEngine->find_path(obj, goal, newpath);

    // insert results into data structures
    if (newpath.locs.size() > 0) {
      tit->init_path(obj, newpath.locs);
    }
    else {
      // fixme: maybe add to "failed"/sleep etc. ?
      tit->noPathFound = true;
    }
    return true;
  }

  bool ST_Terrain::plan_failed_task(void)
  {
    assert(pfEngine!=NULL);

    // find a failed path
    FORALL_B(tasks,tit) {
      if( tit->isFailed )
        break;
    }
    // if none exists
    if( tit == tasks.end() )
      return false;

    // object to move
    assert(tit->task.objs.size() > 0);
    const Object *obj = *(tit->task.objs.begin());

    // if the object is moving, stop it first and come back to it later
    if( obj->get_speed() > 0 || obj->get_moving() ) {
      objsToStop.insert(obj);
      return false;
      // fixme: might be better to try another failed task already in this tick...
    }

    // find-path
    Path newpath;
    TEST(cout << "replanning path" << endl);
    pfEngine->find_path(obj, tit->path->goal, newpath);
    if( newpath.locs.size() > 0 ) {
      tit->path->path.clear();
      tit->path->path.insert(tit->path->path.begin(),newpath.locs.begin(),newpath.locs.end());
      tit->path->current_target = (sint4)newpath.locs.size()-1;
      tit->isFailed = false;
    } else {
      tit->noPathFound = true;
    }
    return true;
  }

  bool ST_Terrain::is_at_location(const Object* obj, const Loc& target)
  {
    Loc position; obj->get_center(position.x, position.y);
    return position.distance(target) <= obj->get_speed();
  }

  bool ST_Terrain::is_at_goal(const Object* obj, const Task& task)
  {
      // Getting to the goal depends on the definition of the task:
      if( task.goal.target == Goal::OBJ ) {
          switch(task.goal.mode) {
            case Goal::TOUCH :
              return( obj->distance_to(*task.goal.obj) <= 1 );
              // using 1-tick look-ahead:
              //return( obj->distance_to(*task.goal_obj) <= obj->get_speed() );
              break;

            case Goal::VICINITY :
              assert(0 && "Not implemented yet");
              break;

            case Goal::ATTACK :
              assert(0 && "Not implemented yet");
              break;

            default:
              ERR("Unknown mode");
          }
      } else if( task.goal.target == Goal::LOCATION ) {
          switch(task.goal.mode) {
            case Goal::TOUCH :
              return is_at_location(obj, task.goal.loc);
              break;

            case Goal::VICINITY :
              assert(0 && "Not implemented yet");
              break;

            case Goal::ATTACK :
              assert(0 && "Not implemented yet");
              break;

            default:
              ERR("Unknown mode");
          }
      } else {
        ERR("Unknown target type");
      }
      return false;
  }


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

#define EPS 1e-9f


  UnitPath::UnitPath(const Object *obj_, const Vector<Loc> &path_)
    : obj(obj_), path(path_), no_orders_yet(true), collision_step(0), wakeup(0)
  {
    current_target = path.size()-1;
    if (path.size() > 0) {
      goal = path[0];
    }
    spot = Loc(-1, -1);
  }


  bool UnitPath::is_intersect(const Segment &segment) const
  {
    typeof(path.begin()) pj = path.begin();
    typeof(path.begin()) pi = pj++;
    for( ; pj != path.end() ; pi++, pj++ ) {
      if( Segment(*pi, *pj).touches(segment) )
        return true;
    }
    return false;
  }

#if 0
  Vector<Segment> UnitPath::intersect(const Segment &segment)
  {
    Vector<Segment> lines;
    const Loc& a = segment.l1;
    const Loc& b = segment.l2;
  
    for (uint4 i = 1; i < path.size(); i++) {

      Loc c = path[i-1];
      Loc d = path[i];
    
      real8 m1 = (real8)(c.y-d.y) / (real8)(c.x-d.x + EPS);
      real8 m2 = (real8)(a.y-b.y) / (real8)(a.x-b.x + EPS);
    
      if ((int)m1 == (int)m2) continue; // fixme: equality for reals doesn't work
    
      real8 b1 = c.y - m1 * c.x;
      real8 b2 = a.y - m2 * a.x;        
    
      real8 X = (b2-b1) / (m1-m2);
      real8 Y = m1*X + b1;
    
      real8 t1x = (real8)(X - a.x) / ((real8)(b.x - a.x + EPS));
      real8 t1y = (real8)(Y - a.y) / ((real8)(b.y - a.y + EPS));
      real8 t2x = (real8)(X - c.x) / ((real8)(d.x - c.x + EPS));
      real8 t2y = (real8)(Y - c.y) / ((real8)(d.y - c.y + EPS));
    
      bool overlap1 = (t1x < 0 || t1x > 1);
      if (!overlap1) continue;
    
      bool overlap2 = (t1y < 0 || t1y > 1);
      if (!overlap2) continue;    
    
      bool overlap3 = (t2x < 0 || t2x > 1);
      if (!overlap3) continue;        

      bool overlap4 = (t2y < 0 || t2y > 1);
      if (!overlap4) continue;    

      lines.push_back(Segment(path[i-1], path[i]));
    }
  
    return lines;
  }
#endif

}   // End of namespace Demo_SimpleTerrain
