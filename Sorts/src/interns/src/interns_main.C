// $Id: sampleai_main.C,v 1.7 2007/03/15 10:37:47 orts_furtak Exp $

// This is an Interns file, made by the John Laird's 2007 summer interns.
// It is roughly based off of the ORTS sampleAI client

// this is the main file of the Interns client
// options are specified by the opt.put lines below
// to test it start the ORTS server (bin/orts) and then run bin/interns [options]

// InternEventHandler.C contains the code for accessing the game state

#include "Global.H"
#include "GfxGlobal.H"
#include "Options.H"
#include "GfxModule.H"
#include "GameStateModule.H"
#include "ShutDown.H"
#include "SDLinit.H"
#include "Game.H"
#include "GUI.H"

#include "InternEventHandler.H"
#include "TerrainModule.H"

#include "SimpleTerrain.H"



typedef SimpleTerrain::ST_Terrain Terrain;

using namespace std;

//===================================================================

static char *VERSION = "sampleai v0.1";

static const sint4 GFX_X = 900;  // graphics window size
static const sint4 GFX_Y = 740;

//===================================================================

void add_options()
{
  Options opt("SampleClient");
  opt.put("-usegfx", "display world in 3d");
  opt.put("-cinfo", "display game state changes");
  opt.put("-tinfo", "display tile info");
  opt.put("-oinfo", "display object info");
  opt.put("-minfo", "display map info");  

  opt.put("-disp",  "2d display");
}

//===================================================================

// loop is called

class Looper : public LoopFunctor
{
public :

  Looper(SampleAIState *state_) : state(state_) {}
  
  void loop() const {

    if (state->gfxm) {

      if (state->gfxm->get_quit()) { state->quit = true; exit(0); } // should return

      bool refresh;
      Options::get("-refresh", refresh);
      
      if (refresh) {

	// maximal gfx refresh rate
	
	if (!state->just_drew) 
	  state->gfxm->draw();
	else
	  state->just_drew = false;
      }
    }
  
    // looks for server messages
    // if one or more arrived, calls handlers
    // if not, sleeps for one millisecond
    if (!state->gsm->recv_view()) SDL_Delay(1);
  }

private:
  SampleAIState *state;
};

//===================================================================

int main(int argc, char **argv)
{
  signals_shut_down(true);  
  
  // populate opt with options from various modules

  add_options();
  GameStateModule::Options::add();
  GfxModule::Options::add();
  Game::Options::add();
  GUI::add_options();
  Terrain::add_options();

  // process command line arguments
  if (Options::process(argc, argv, cerr, VERSION)) exit(20);

  // initialize SDL
  SDLinit::video_init();
  SDLinit::network_init();
  
  // populate option objects with command line values
  // and create modules
  
  GameStateModule::Options gsmo;
  GameStateModule gsm(gsmo);
 
  GfxModule::Options gfxmo;
  GfxModule *gfxm = 0;

  // change seed value to time(0) if 0

  sint4 seed;
  Options::get("-seed", seed);
  if (!seed) seed = time(0);
  Options::set("-seed", seed);
  cout << "seed=" << seed << endl;

  // debug graphics

  GUI *gui = 0;
  bool disp;
  Options::get("-disp", disp);
  if (disp) {
    SDLinit::video_init();
    gui = new GUI;
  }

  // 3d graphics

  bool use_gfx;
  Options::get("-usegfx", use_gfx);

  if (use_gfx) gfxm = new GfxModule;

  //This establishes the pathfinding map
  Terrain timp;
  TerrainModule tm(gsm, timp,1000000000);
  
  // gives event handler access to the state
  SampleAIState state(&gsm, gfxm, gui);

  InternEventHandler sev(state, seed, &tm);

  // gsm events are handled in sev
  gsm.add_handler(&sev);
  
  
  
  // connect game state module to server
  if (!gsm.connect()) ERR("connection problems");

  if (gui) {
    gui->init(GFX_X, GFX_Y, gsm.get_game());
    gui->display();
  }

  Looper l(&state);

  if (use_gfx) {

    // gfx mode

    glutInit(&argc, argv);
    gfxm->init(gsm.get_game(), gsm.get_changes(), gsm.get_action_changes(), gfxmo);

    // game loop (could become a thread)
    gfxm->start_loop(&l);

  } else {

    // no gfx
    
    while (!state.quit) l.loop();
  }
  
  delete gfxm;
  return 0;
}
