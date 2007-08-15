// FORS/FORALL/ERR macros are defined in orts3/libs/kernel/src/Global.H

#include "InternEventHandler.H"
#include "GameStateModule.H"
#include "GfxModule.H"
#include "Game.H"
#include "GameChanges.H"
#include "GameTile.H"
#include "ServerObjData.H"
#include "Options.H"
#include "GUI.H"

#include "DrawOnTerrain2D.H"

using namespace std;

// tile types from core.bp
// fixme: game script reflection would help here
// in the competition we won't use water tiles
// TILE_CLIFF: non-traversable by ground units

enum { TILE_UNKNOWN=0, TILE_WATER=1, TILE_GROUND=2, TILE_CLIFF=3 };


/************************************************************************
 * handle_event
 ************************************************************************/
bool InternEventHandler::handle_event(const Event& e)
{
  if (e.get_who() == GameStateModule::FROM) {
    
    //cout << "Event: " << e.who() << " " << e.what() << endl;
    
    if (e.get_what() == GameStateModule::STOP_MSG) {
      state.quit = true; return true;
    }
    
    if (e.get_what() == GameStateModule::READ_ERROR_MSG) {
      state.quit = state.error = true; return true;
    }
    
    if (e.get_what() == GameStateModule::VIEW_MSG) {
      DrawOnTerrain2D dot(state.gui);
      dot.start();

      compute_actions(&dot);
      state.gsm->send_actions();

      Game &game = state.gsm->get_game();	

      // gfx and frame statistics
      
      if (state.gfxm) state.gfxm->process_changes();
      
      sint4 vf = game.get_view_frame();
      sint4 af = game.get_action_frame();
      sint4 sa = game.get_skipped_actions();
      sint4 ma = game.get_merged_actions();

      if ((vf % 20) == 0) cout << "[frame " << vf << "]" << endl;
	
      if (vf != af || sa || ma > 1) {
            
	cout << "frame " << vf;
	if (af < 0) cout << " [no action]";
	else if (vf != af) cout << " [behind by " << (vf - af) << " frame(s)]";
	if (sa) cout << " [skipped " << sa << "]";
	if (ma > 1) cout << " [merged " << ma << "]";
	cout << endl;
      }

      if (state.gfxm) {
	
	// don't draw if we are far behind
	//if (abs(vf-af) < 10) {
	  state.gfxm->draw();
	  state.just_drew = true;
	//}
      }

      if (state.gui) {
        state.gui->event();
        //        state.gui->display();
        if (state.gui->quit) exit(0);
      }

      // as a hack for flickering, gui->display() is called in dot.start()
      // so call everything that draws to the debug window between start() and end()
      dot.draw_line(-60, 10, -20, 10, Vec3<real4>(1.0, 1.0, 1.0));
      dot.draw_line(-60, 10, -65, 15, Vec3<real4>(1.0, 1.0, 1.0));
      dot.draw_line(-44, 10, -56, 40, Vec3<real4>(1.0, 1.0, 1.0));
      dot.draw_line(-28, 10, -36, 40, Vec3<real4>(1.0, 1.0, 1.0));
      dot.draw_line(-36, 40, -30, 34, Vec3<real4>(1.0, 1.0, 1.0));

      dot.draw_circle(-40, 25, 35, Vec3<real4>(1.0, 0.0, 1.0));

      //Draw influence maps
      bool inf_inf, inf_walk, inf_ore;
      Options::get("-cinfo", inf_inf);
      Options::get("-minfo", inf_walk);
      Options::get("-oinfo", inf_ore);
      
      if (inf_inf)
	influence->draw(&dot);
      if (inf_walk)
	walkability->draw(&dot);
      if (inf_ore)
	;


      dot.end();

      return true;
    }
  }
    
  return false;
}


/************************************************************************
 * compute_actions - Main event handling function
 ************************************************************************/
void InternEventHandler::compute_actions(DrawOnTerrain2D *dot)
{
  const GameChanges &changes = state.gsm->get_changes();
  const Game &game = state.gsm->get_game();
  const sint4 cid = game.get_client_player();
  const Map<GameTile> &map = game.get_map();
  const Game::ObjCont &objs = game.get_objs(cid);
  const sint4 points_x = map.get_width()  * game.get_tile_points();
  const sint4 points_y = map.get_height() * game.get_tile_points();  
 
  cout << endl;
  cout << "compute actions" << endl;


  // ***************** Debugging Statements ******************
  printCinfo();
  printTerrain();
  printObjects();

  // ***************** Create Walkability Map ****************
  if (game.get_view_frame() == 0){
    walkability = new InfluenceMap(points_x, points_y, game.get_tile_points());
    walkability->init_map(&game);
  }

  // ***************** Create Influence Map ******************
  if (game.get_view_frame() == 0){
    influence = new InfluenceMap(points_x, points_y, game.get_tile_points());
    influence->init_game(&game, cid);
  }
  
  influence->refresh_units(&game, cid);

  // ***************** Check for New Units -- Place in Squads ****
  if(changes.new_objs.size() > 0)
    squadManager.insertUnits(changes.new_objs, cid);
  if(changes.dead_objs.size() > 0)
    squadManager.removeUnits(changes.dead_objs, cid);

  // *************** Move Units ***********  
  squadManager.destroy_bases(influence);
  //squadManager.worker_rush(influence);  

// ******************
  // Attack enemy if within range
  FORALL (objs, it) {
    GameObj *gob = (*it)->get_GameObj();
    
    if (gob && gob->sod.in_game && *gob->sod.x >= 0 && !gob->is_dead()) {
      ScriptObj *weapon = gob->component("weapon");
      if (!weapon) continue;
      const sint4 n = game.get_player_num();
      GameObj *target = 0;
      
      if (n > 1) { // there are other players
	sintptr *prange = weapon->get_int_ptr("max_ground_range");
	if (!prange) continue;
	const sint4 range2 = *prange * *prange;


	// For all players	
	FORS (i, n) {
	  if (i == cid) continue;
	  
	  // check for targets (could be done much faster)
	  const Game::ObjCont &opp_objs = game.get_objs(i);
	  
	  FORALL (opp_objs, it){
	    GameObj *gob2 = (*it)->get_GameObj();
	    if (gob2 && gob2->sod.in_game && *gob2->sod.x >= 0 && !gob2->is_dead()){
	      real8 d2 =
		square(real8(*gob->sod.x - *gob2->sod.x)) +
		square(real8(*gob->sod.y - *gob2->sod.y));
	      
	      if (d2 <= range2 + (4 * *gob->sod.radius)){
		// Fire first on workers marines and tanks
		if (gob2->bp_name() == "marine" || 
		    gob2->bp_name() == "tank" || 
		    gob2->bp_name() == "worker"){
		  
		  // If there is no target, assign one immediately
		  if (!target){
		    target = gob2;
		  }
		  // If we're shooting at a target and not hitting it, switch targets
		  else  if (target == gob2 && !gob->component("weapon")->get_int("shooting")){
		    target = NULL;
		  }
		  // fire on those with the lowest health
		  else  if (target && gob2->get_int("hp") < target->get_int("hp")){
		    target = gob2;
		  }
		}
		// If there are no better targets, fire on the barracks
		else if (gob->bp_name() == "barracks" && !target)
		  target=gob2;
		// If there are no better targets, fire on control centers
		else if (gob2->bp_name() == "controlCenter" &&
			 !target)
		  target = gob2;
	      }
	    }
	  }
	}
      }
     
      
      // Found something worth firing at
      if (target) {
	if (gob->component("weapon")->get_int("shooting") && 
	    game.get_view_frame() % 5 == 0)
	  dot->draw_line(*gob->sod.x, *gob->sod.y, *target->sod.x, 
			 *target->sod.y, Vec3<real4>(0.8, 1.0, 0.8));
	else if (gob->component("weapon")->get_int("shooting")) 
	  dot->draw_line(*gob->sod.x, *gob->sod.y, *target->sod.x, 
			 *target->sod.y, Vec3<real4>(1.0, 1.0, 1.0));
	else 
	  dot->draw_line(*gob->sod.x, *gob->sod.y, *target->sod.x, 
			 *target->sod.y, Vec3<real4>(1.0, 0.0, 0.0));
	
	// Fire at it
	if (game.get_view_frame() % 5 == 0){
	  Vector<sint4> args;
	  args.push_back(game.get_cplayer_info().get_id(target));
	  gob->component("weapon")->set_action("attack", args);
	}
      }
    }
  }
}


/************************************************************************
 * Cinfo - Print all of the changes between this cycle and last
 ************************************************************************/
void InternEventHandler::printCinfo() {
  const GameChanges &changes = state.gsm->get_changes();
  bool cinfo;
  Options::get("-cinfo", cinfo);
  
  // Flag -cinfo
  if (cinfo){

    // - what changed in the last simulation tick
    if (!changes.new_tile_indexes.empty())
      cout << "#new tiles = " << changes.new_tile_indexes.size() << endl;
  
    if (!changes.new_boundaries.empty())
      cout << "#new boundaries = " << changes.new_boundaries.size() << endl;

    // fixme: what about removed tiles and removed boundaries?
    // currently tiles and boundaries are never removed
    
    if (!changes.new_objs.empty())
      cout << "#new objects = " << changes.new_objs.size() << endl;

    if (!changes.changed_objs.empty())
      cout << "#changed objects = " << changes.changed_objs.size() << endl;

    if (!changes.vanished_objs.empty())
      cout << "#vanished objects = " << changes.vanished_objs.size() << endl;
    
    if (!changes.dead_objs.empty())
      cout << "#dead objects = " << changes.dead_objs.size() << endl;

  }
}

/************************************************************************
 * Terrain - Print information about the map assuming flags are set 
 ************************************************************************/
void InternEventHandler::printTerrain() {
  const Game &game = state.gsm->get_game();
  const Map<GameTile> &map = game.get_map();
  const GameChanges &changes = state.gsm->get_changes();

  bool minfo;
  Options::get("-minfo", minfo);
  
  if (minfo) {
    
    // - map dimensions
    
    cout << "Map width: = " << map.get_width()  << " tiles" << endl;
    cout << "Map height: " << map.get_height() << " tiles" << endl;  
    
    // height field scale: HEIGHT_MULT = one tile width up, constant for now
    cout << "height mult: " << GameConst::HEIGHT_MULT << endl; 
    
    // - new/updated tile indexes
    
    cout << "# of new/updated tiles " << changes.new_tile_indexes.size() << endl;
    cout << "Index(es): ";
    FORALL (changes.new_tile_indexes, it) cout << *it << " ";
    cout << endl;
    
    // - new boundaries
    
    const Vector<ScriptObj::WPtr> &boundaries = changes.new_boundaries;
    
    FORALL (boundaries, it) {
      
      const GameObj *g = (*it)->get_GameObj();
      if (g == 0) continue;
      
      cout << "new boundary (" << *g->sod.x1 << "," << *g->sod.y1 << ")-("
	   << *g->sod.x2 << "," << *g->sod.y2 << ")"
	   << endl;
    }
    
    // - all boundaries
    
    const Game::ObjCont &all_boundaries = game.get_boundaries();
    cout << "total # of boundaries = " << all_boundaries.size() << endl;
  }
  
  
  bool tinfo;
  Options::get("-tinfo", tinfo);
  if (tinfo) {
    
    // - all tiles
    
    FORS (y, map.get_height()) {
      FORS (x, map.get_width()) {
	const GameTile &tile = map(x,y);
	cout << "[" << x << "," << y << "]: ";
	
	sint4 hNW, hNE, hSE, hSW;
	Tile::Split split;
	
	// get corner heights and split type
	tile.get_topo(hNW, hNE, hSE, hSW, split);
	
	// tile types: see enum on top
	// split types: see orts3/libs/kernel/src/Tile.H
	// if split == Tile::NO_SPLIT => typeW is the tile type
	
	cout << "split:" << split 
	     << " type w:" << tile.get_typeW()
	     << " type e:" << tile.get_typeE()
	     << " heights:" << hNW << " " << hNE << " " << hSE << " " << hSW
	     << endl;
      }
      
      cout << endl;
    }
    
    cout << endl;
  }
  
  
  // - currently visible tiles
  
  // ... to be implemented
}
/************************************************************************
 * printObjects - Print information about all of the game objects
 ************************************************************************/
void InternEventHandler::printObjects() {
  const Game &game = state.gsm->get_game();
  const sint4 cid = game.get_client_player();

  bool oinfo;
  Options::get("-oinfo", oinfo);
  if (oinfo){
    // units under client player control
    
    const Game::ObjCont &objs = game.get_objs(cid);
    
    FORALL (objs, it) {
      
      const GameObj *gob = (*it)->get_GameObj();
      if (!gob) ERR("not a game object!?");
      
      const ServerObjData &sod = gob->sod;
      
      // - fundamental properties
      
      // blueprint name ~ type name
      cout << "bp: " << gob->bp_name();

      if (!sod.in_game) { cout << " no gameobj" << endl; continue; }

      if (*sod.x < 0) { cout << "not on playfield" << endl; continue; }
      
      // object on playfield

      // sod.shape and sod.zcat values defined in orts3/libs/kernel/src/Object.H
      
      cout << "zcat: " << *sod.zcat
	   << " max-speed: " << *sod.max_speed
	   << " speed: " << *sod.speed 
	   << " moving: " << *sod.is_moving 
	   << " sight: " << *sod.sight;
      
      if (*sod.shape == Object::CIRCLE) {
	cout << " CIRCLE loc: (" << *sod.x << "," << *sod.y << ")"
	     << " r: "<< *sod.radius;
	  
      } else if (*sod.shape == Object::RECTANGLE) {
	cout << " RECT dim: (" << *sod.x1 << "," << *sod.y1 << ")-("
	     << *sod.x2 << "," << *sod.y2 << ")";
      } else {
	cout << " UNKNOWN SHAPE";
      }

      // - other object properties

      // heading: 0 = stopped, 1...GAME_CONST::HEADING_N
      const sintptr *hd = gob->get_int_ptr("heading");
      if (hd) cout << " hd: " << GameConst::angle_from_dir(*hd, GameConst::HEADING_N);

      // ... more examples
      
      cout << endl;
    }
  }

  // - what an object currently sees

  // ... to be implemented
}
