#ifndef InternEventHandler_H
#define InternEventHandler_H

// $Id: SampleEventHandler.H,v 1.2 2007/03/15 10:37:47 orts_furtak Exp $

// This is an ORTS file (c) Michael Buro, licensed under the GPL

#include "EventHandler.H"
#include "Random.H"
#include "Vector.H"
#include "GameChanges.H"
#include "SquadManager.H"
#include "TerrainModule.H"
#include "InfluenceMap.H"

#include "DrawOnTerrain2D.H"
class GameStateModule;
class GfxModule;
class GUI;

// This class gives the event handler access to all game and gfx data

struct SampleAIState {

  GameStateModule *gsm;
  GfxModule       *gfxm; // !=0 : gfx used
  GUI             *gui;  // !=0 : debug gfx used
  bool just_drew;  // just drew a gfx frame
  bool quit;       // true => quit game
  bool error;      // true <=> error occurred
  
  SampleAIState(GameStateModule *gsm_, GfxModule *gfxm_, GUI *gui_) :
    gsm(gsm_), gfxm(gfxm_), gui(gui_)
  {
    just_drew = quit = error = false;
  }

};


// This class handles server events
// In particular it generates game object actions

class InternEventHandler : public EventHandler {

public:
  
  SampleAIState &state;
  InfluenceMap *influence;
  InfluenceMap *walkability;

  InternEventHandler(SampleAIState &state_, sint4 seed, TerrainModule *map_) : state(state_){
    rand.srand(seed);
    terrainMap = map_;
  }

  bool handle_event(const Event &e);

  TerrainModule *terrainMap;
private:
  
  void compute_actions(DrawOnTerrain2D *dot);
  SquadManager squadManager;
  Random rand; // local RNG

  void printObjects();
  void printTerrain();
  void printCinfo();

    
};

#endif
