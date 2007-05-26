#ifndef SortsTerrainModuleh
#define SortsTerrainModuleh

// $Id: SortsTerrainModule.H,v 1.9 2006/06/09 12:48:47 ddeutscher Exp $
// This is an ORTS file (c) Michael Buro, David Deutscher, licensed under the GPL

// glue between ORTS and terrain code
// event-driven interface

#include "GameStateModule.H"
#include "SortsSimpleTerrain.h"

class GameObj;

//===================================================================

class SortsTerrainModule : public ClientModule, public EventHandler
{
  public:
    void findPath(GameObj* gob, SortsSimpleTerrain::Loc goal, SortsSimpleTerrain::Path& path);
    void findPath(SortsSimpleTerrain::Loc start, SortsSimpleTerrain::Loc goal, SortsSimpleTerrain::Path& path);
    void findPath(GameObj* gob, GameObj* goal, SortsSimpleTerrain::Path& path);
    void insertImaginaryWorker(SortsSimpleTerrain::Loc l);
    void removeImaginaryWorker(SortsSimpleTerrain::Loc l);
    void insertDynamicObjs();
    void removeDynamicObjs();
    void insertControlCenters();
    void removeControlCenters();



    static const sint4 FROM;
    static const sint4 FIND_PATH_MSG;
    static const sint4 FIND_PATH_STOP;
    static const sint4 PATH_SUCCESS;
    static const sint4 PATH_FAILURE;
    static const sint4 PATH_PLAN_FAILURE;
    static const sint4 PATH_MOVE_FAILURE;
    static const sint4 MOVE_PRIORITY;///< Priority of Move commands

    /** Initialize the SortsTerrainModule with some implementation of the SortsSimpleTerrain 
      interface. Optionally give a bound on the maximum time (in milliseconds)
      spent by the implementation on planning per each game tick 
      (0 = automatic limit). */
    SortsTerrainModule(GameStateModule &gsm, SortsSimpleTerrain::SortsST_Terrain &timp, sint4 max_ms_per_tick);
    ~SortsTerrainModule();

    bool handle_event(const Event &e);

  private:

    /// apply set_action and send arrived/failed events
    void act(void);
    /// call the implementations' methods for view changes
    void notify_timp_on_world_changes(const GameChanges &changes);

    GameStateModule &gsm;
    SortsSimpleTerrain::SortsST_Terrain &timp;        ///< terrain implementation
    sint4 max_ms_per_tick;    ///< max time (milliseconds) to be spent by timp on planning in each game tick

};

//===================================================================

#endif
