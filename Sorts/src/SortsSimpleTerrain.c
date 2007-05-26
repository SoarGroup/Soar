// $Id: SortsSimpleTerrain.C,v 1.9 2006/06/09 18:46:14 ddeutscher Exp $

// This is an ORTS file 
// (c) Michael Buro, Sami Wagiaalla, David Deutscher
// licensed under the GPL

#include "SortsSimpleTerrain.h"
#include "SortsST_PFEngine.h"
#include "Global.H"
#include "Options.H"

using namespace std;

#define TEST(x)
//#define TEST(x) x

namespace SortsSimpleTerrain {
  // Sorts additions
  void SortsST_Terrain::findPath(const Object* gob, const Loc &l2, Path &path) {
    pfEngine->find_path(gob, l2, path);
  }
  void SortsST_Terrain::findPath(const Loc &l1, const Loc &l2, Path &path) {
    pfEngine->find_path(l1, l2, path);
  }
  void SortsST_Terrain::insertImaginaryWorker(Loc l) {
    pfEngine->insertImaginaryWorker(l);
  }
  void SortsST_Terrain::removeImaginaryWorker(Loc l) {
    pfEngine->removeImaginaryWorker(l);
  }
  void SortsST_Terrain::removeDynamicObjs() {
    pfEngine->removeDynamicObjs();
  }
  void SortsST_Terrain::insertDynamicObjs() {
    pfEngine->insertDynamicObjs();
  }
  void SortsST_Terrain::removeControlCenters() {
    pfEngine->removeControlCenters();
  }
  void SortsST_Terrain::insertControlCenters() {
    pfEngine->insertControlCenters();
  }
void SortsST_Terrain::findPath(const Object* gob, const Object* l2, Path &path) {
    sint4 x1, y1; l2->get_center(x1, y1);
    Loc l;
    l.x = x1;
    l.y = y1;
    sint4 x2, y2; gob->get_center(x2, y2);
    Loc lg;
    lg.x = x2;
    lg.y = y2;
    pfEngine->remove_object(l2);
    //pfEngine->clearGobLocation(l2);
    pfEngine->remove_object(gob);
    //pfEngine->clearGobLocation(gob);
    pfEngine->find_path(lg, l, path);
    pfEngine->insert_object(l2);
    pfEngine->insert_object(gob);
  }


  //-----------------------------------------------------------------------------
  void SortsST_Terrain::add_options()
  {
    ::Options o("SimpleTerrain");
    o.put("-gran",  sint4(4),  "granularity of the fine grid default 4x4");
    o.put("-costonly", "represent boundaries only by higher edge cost");
  }

  //-----------------------------------------------------------------------------
  void SortsST_Terrain::init(sint4 tiles_x_, sint4 tiles_y_, sint4 tile_points_, sint4 me, sint4 neutral)
  {
 //   SortsTerrainBasicImp<SortsST_Task>::init(tiles_x_, tiles_y_, tile_points_, me, neutral);
    sint4 gran;
    Options::get("-gran", gran);
    pfEngine = boost::shared_ptr<SortsPFEngine>(new SortsPFEngine(tiles_x_, tiles_y_, tile_points_, gran));
  }

  //-----------------------------------------------------------------------------
  void SortsST_Terrain::add_obj(const Object *obj)
  {
 //   SortsTerrainBasicImp<SortsST_Task>::add_obj(obj);
    assert(pfEngine!=NULL);
    pfEngine->insert_object(obj);
  }

  //-----------------------------------------------------------------------------
  void SortsST_Terrain::update_obj(const Object *obj)
  {
 //   SortsTerrainBasicImp<SortsST_Task>::update_obj(obj);
    assert(pfEngine!=NULL);
    pfEngine->update_object(obj);
  }

  //-----------------------------------------------------------------------------
  void SortsST_Terrain::remove_obj(const Object *obj)
  {
    // remove from world representation
    assert(pfEngine!=NULL);
    pfEngine->remove_object(obj);

  }

  //-----------------------------------------------------------------------------
  void SortsST_Terrain::add_segments(const Vector<Segment> &segs)
  {
    assert(pfEngine!=NULL);
    FORALL(segs,i)
      pfEngine->insert_boundary(*i);
  }

}   // End of namespace SortsSimpleTerrain
