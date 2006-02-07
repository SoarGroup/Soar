// $Id: orts_main.C,v 1.30 2005/11/16 19:17:46 orts_furtak Exp $

// This is an ORTS file (c) Michael Buro, licensed under the GPL
 
#include "Global.H"
#include "Game.H"
#include "GUI.H"
#include "CycleCounter.H"
#include "Options.H"
#include "Server.H"
#include "GameStateModule.H"
#include "Compress.H"
#include "ShutDown.H"
#include "SDLinit.H"
#include "MapTool.H"

using namespace std;

const char *ORTS_VERSION = "ORTS version 3.1 (2005-May-16)";

#define USE_EFENCE 0

extern int EF_PROTECT_FREE, EF_FILL, EF_PROTECT_BELOW, EF_ALLOW_MALLOC_0;
 
static const sint4 GFX_X = 900;  // graphics window size
static const sint4 GFX_Y = 740;



static void add_options()
{
  Options o("ORTS");
  o.put("-loadmap", string(""),      "map file name to load");
  o.put("-newt",    true,            "new terrain");
  o.put("-debug",   false,           "debug mode");
 
// FRANO
  o.put("-frano",false,"frano's test map");
  
  o.put("-soar", string (""), "soar test map");
  o.put("-marines", "generate marines");  
  
  o.put("-game1",   "tournament game 1");
  o.put("-game2",   "tournament game 2");
  o.put("-game3",   "tournament game 3");
  
  o.put("-disp",    "2d display");

  o.put("-m",       string("server"),"mode: server|client|game|test|worker");
  o.put("-showv",   false,           "show views");
  o.put("-showl",   false,           "show message length");

  o.put("-hfile",   string(""),      "create map from height/ramp file");
  o.put("-out",     string(""),      "save map to file");

}


void create_orts_world(stringstream &world)
{
  // note: if loading a map, -nplayers values must match
  sint4 num_players;
  Options::get("-nplayers", num_players);
  
  // note: current design is for the map to contain the blueprint,
  // thus ignoring any -bp value on the orts command-line.
  // If there is no map specified, the -bp value is used.
  string map_fname;
  Options::get("-loadmap", map_fname);
  
  if (map_fname.empty()) {

    string hfile;
    Options::get("-hfile", hfile);
    if (!hfile.empty()) {
      
      // read height map + ramp information from file
      
      MapTool::read_height_map(world);

    } else {

      // default behaviour - create random world using seed value

      bool newt, debug, marines;
      bool game1, game2, game3;

      sint4 island;
      Options::get("-newt", newt);
      Options::get("-debug", debug);
      Options::get("-island", island);
      Options::get("-marines", marines);      
      Options::get("-game1", game1);
      Options::get("-game2", game2);
      Options::get("-game3", game3);

      //FRANO ---
      bool frano = false;
      Options::get("-frano", frano);
      // ---
      //soar ---
      string soar = "";
      Options::get("-soar",soar);
      // ---
      if      (game1)   MapTool::generate_game1_map(world);
      else if (game2)   MapTool::generate_game2_map(world);
      else if (game3)   MapTool::generate_game3_map(world);

      else if (marines)
        MapTool::generate_marine_map(world);
      else if (frano) {
        MapTool::generate_frano_map(world);
      } else if (soar != "")
        MapTool::generate_soar_map(world, soar.c_str());
      else if (island)
        MapTool::generate_island_map(world);
      else if (debug)
        MapTool::generate_debug_map(world);
      else if (newt)
        MapTool::generate_random_cliff_map(world);
      else
        MapTool::generate_random_map(world);
    }
    
  } else {

    // load world from map file
    cout << "loading world from map file: " << map_fname << endl;

    ifstream map_file(map_fname.c_str());
    if (!map_file.is_open()) ERR ("file not found, cannot load map");

    while (map_file.good()) {
      char ch;
      map_file.get(ch);
      world << ch;
    }
  
    map_file.close();
  }
}


struct ClientEventHandler : public EventHandler
{
  GameStateModule *gsm;
  GUI *gui;
  
  ClientEventHandler(GameStateModule *gsm_, GUI *gui_) : gsm(gsm_), gui(gui_) { }
  
  bool handle_event(const Event &e)
  {
    if (e.get_who() == GameStateModule::FROM) {

      if (e.get_what() == GameStateModule::STOP_MSG) exit(0);
      
      if (e.get_what() == GameStateModule::VIEW_MSG) {

	cout << "." << flush;
	
	if (gui) {
	  gui->event();
	  gui->display();
	  if (gui->quit) exit(0);
	}

	gsm->send_actions();
	
	return true;

      } else errstr << "unhandled event: from "
		    << e.get_who() <<  " : "
		    << e.get_what() << endl;
    }

    return false;
  }

};


struct ServerEventHandler : public EventHandler
{
  Server *server;
  GUI *gui;
  bool init;
  
  ServerEventHandler(Server *server_, GUI *gui_) :
    server(server_), gui(gui_), init(false) {}
  
  bool handle_event(const Event& e)
  {
    if (gui && !init) {
      init = true;
      gui->init(GFX_X, GFX_Y, server->get_game());
    }
    
    if (e.get_who() == Server::FROM) {

      if (e.get_what() == Server::STEP_MSG) {

	if (gui) {
	  gui->event();
	  gui->display();
	  if (gui->quit) exit(0);
	}
       
	return true;

      } else
	
	errstr << "unhandled event: from "
	       << e.get_who() <<  " : " << e.get_what()
	       << endl;
    }
    return false;
  }

};


int main(int argc, char **argv)
{
 try {
#if USE_EFENCE
  // aggressive efence checks
  EF_PROTECT_FREE = 1;
  EF_FILL = 0xee;
  EF_PROTECT_BELOW = 1;
  EF_ALLOW_MALLOC_0 = 0;
#endif

  signals_shut_down(true);

  add_options();
  MapTool::Options::add();
  GameStateModule::Options::add();
  Server::Options::add();
  GUI::add_options();

  if (Options::process(argc, argv, cerr, ORTS_VERSION)) exit(20);

  // change seed value to time(0) if 0
  sint4 seed;
  Options::get("-seed", seed);
  if (!seed) seed = time(0);
  Options::set("-seed", seed);
  cout << "seed=" << seed << endl;

  GUI *gui = 0;
  bool disp;
  Options::get("-disp", disp);
  if (disp) {
    SDLinit::video_init();
    gui = new GUI;
  }
 
  string mode;
  Options::get("-m", mode);
  
  if (mode == "server" || mode == "client")
    SDLinit::network_init();

  if (mode == "client") {

    GameStateModule::Options gsmo;
    GameStateModule gsm(gsmo);
    ClientEventHandler ceh(&gsm, gui);

    gsm.add_handler(&ceh);
    
    if (!gsm.connect()) exit(10);

    if (gui) {
      gui->init(GFX_X, GFX_Y, gsm.get_game());
      gui->display();
    }

    while (!shut_down) {
      if (!gsm.recv_view()) SDL_Delay(1);
    }
    
    return 0;
  }
  stringstream world; 
  create_orts_world(world);

  if (mode == "server") {

    Server server;
    ServerEventHandler sev(&server, gui);
    server.add_handler(&sev);

    Server::Options so;
    server.run(world.str(), so);
    return 0;
  }

#if 0
  if (mode == "worker") {

    Net_Worker worker(world.str(), opt);
    worker.run();
    return 0;
  }
#endif
  
  if (mode != "game") ERR("unknown mode");

  Game::Options go;
  Game game(world.str(), go);
  if (gui) gui->init(GFX_X, GFX_Y, game);
  
  // local test
 
  Vector<string> views;
  Vector<string> actions;
  sint4 num_players;
  Options::get("-nplayers", num_players);
  
  FORS (i, num_players) views.push_back(string(""));
  FORS (i, num_players) actions.push_back(string(""));

  sint4 clev, wait;
  sint4 tick = 1, tick_max;
  Options::get("-clev", clev);
  Options::get("-tmax",  tick_max);
  Options::get("-wait",  wait);

  sint4 q = 0;

  uint4 start = SDL_GetTicks();
  
  FOREVER {
    
    ++tick;
    if (shut_down) break;
    if (tick_max > 0 && tick > tick_max) break;

    if (gui) {
      gui->event();
      gui->display();
      if (gui->quit) break;
    }
    
    game.simulation_step(actions, views);

    if ((tick & 15) == 0) {
      cout << tick << " "
	   << tick/((SDL_GetTicks() - start)/1000.0+0.001)
	   << " fps" << endl;
    }
    
    bool act;
    Options::get("-act", act);
    if (act) game.gen_actions(actions);
    
    if (clev > 0) {

      // compress views
      
      FORS (p, num_players) {
        Compress compress(clev);

        string cs;

        compress.doit(views[p], cs);
        views[p].clear();
        views[p] = cs;
      }
    }

    bool showv, showl;
    Options::get("-showv", showv);
    Options::get("-showl", showl);
    
    if (showv || showl) {
      FORS (p, num_players) {
	cout << p << " ";
	//	if (clev == 0 && !showl && showv) {
	//	  cout << endl << views[p] << endl;
	//	  cout << "-------------------------------------" << endl;
	//	} else {
	cout << views[p].size() << endl;
	//      }
      }
    }

    if (wait < 0) {
      if (q > 0) {
	q--;
      } else {
	sint4 s = getchar();
        //	char s;
        //	cin.get(s);
        if (s == ' ') q += 5;
        if (s == 'q') break;
      }
      //      char s[100]; fgets(s, 80, stdin); // wait for <enter>
    } else if (wait > 0) {
      SDL_Delay(wait);                         // wait time
    }
  }

  cout << "game finished" << endl;
 } 
 catch (ExitException& ee)
 {
 	errstr << std::endl << *(ee.get_reason()) << std::endl;
#ifdef GCC
    abort();
#else
 	ABORT;
#endif
 	exit(20);
 }
 return 0;
}
