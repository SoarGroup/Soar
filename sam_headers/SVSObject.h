
// This class is the basic object type in the system. Wm3 should be
// encapsulated within here and pLTM, only accessed by the GUI and GroundedObject.

class SVSObject {
  friend class SpatialScene; // see setGlobalTransformation()
public:
  SVSObject(GroundedObject* source, SVSObject* _parent, string _id, string _classId, bool _imagined);
  SVSObject(SpatialPtr source, SVSObject* _parent, string _id, string _classId, bool _imagined);
    
  ~SVSObject();

  bool isImagined() {
    return imagined;
  }

  bool isPrimitive() {
    return primitive;
  }

  // getter for id/classid goes here

  SVSObject* getParent() {
    return parent;
  }

  // any SVSObject may optionally have a name, which is an arbitrary string
  // assigned to it. This is different from the id, which is a string that
  // never makes it to WM. The name is used during imagery, the agent can
  // specify a name for every object in the generate command, and the name of
  // the resulting object will appear in the metainfo in
  // spatial-scene.contents.object
  string getName();
  void setName(string nm);

  // These add and remove child nodes, including handling linking within the
  // Wm3 scene graph.
  bool addChild(SVSObject* child); // return false if this is primitive 
  bool removeChild(SVSObject* child); // return false if child not present

  // This returns an error if the object is already inhibited at a higher level
  // (but then, so should all commands referencing the object below that level)
  bool inhibit(int level);
  
  // Level is necessary here due to glitching. Object should only be actually
  // uninhibited if the level passed is the lower than the current inhibit
  // level (but this is NOT an error).
  // For example, if the agent inhibits at level 3,
  // and then again at level 2, at the next decision, support for the level-3 inhibit
  // command will go away (since the object is no longer at level 3), and
  // uninhibit will be called by that command's watcher. But the level 2
  // command is still valid, so it should remain inhibited.
  bool uninhibit(int level);
  int getInhibitionLevel();

  vector< SVSObject* > getChildren(); // needed to parse the tree

  // this will generate the appropriate grounded object if it
  // isn't current, or return it if it is
  //
  // this object owns the dynamically allocated polyhedron
  // the pointer is guaranteed valid for the duration of the current decision,
  //
  // The goalLevel is necessary, see below for why.
  GroundedObject* getGroundedObject(string interpretation, int goalLevel);

  // GroundedObjects are goal-level specific, since lower subgoals may add
  // structure that is invisible to higher subgoals (e.g., object parts)

  // Set the texture of this object to be that of the source.
  // Return false if either this or the other isn't primitive.
  bool setTexture(SVSObject* source);

  // This causes the object to assume the intrinsic reference frame of another
  // object. That is, the local transform is copied from the other object, and
  // the shape coordinates or the local transforms of the children are
  // manipulated such that the grounded locations of everything remain
  // unchanged.
  //
  // This is useful for since predicate projection operations (e.g., a convex
  // hull) have no meaningful way of determining why one reference frame should
  // be used over another (what is the "front"?), so the agent can choose one
  // instead (e.g., imagine the convex hull two objects, and give it the FOR of
  // one)
  void assumeReferenceFrameOf(SVSObject* other);
  
  // This clones the tree from this object to below, creating new SVSObjects
  // that the caller has ownership of. This actually returns a set of
  // SVSObject*s, the tree must be parsed to get them all. Caller owns all of
  // them.
  //
  // pLTM retrievals, for example, use this:
  // SVSObject* myNewSceneObject = SVSObject::clone(perceptualLTM->retrieve("something"));
  static SVSObject* clone(SVSObject* source);

  // interface to Wm3 bounding collision checker
  // implementation (from SVS1):
  // wm3Unit->UpdateGS();
  // other->wm3Unit->UpdateGS();
  // return wm3Unit->WorldBound->TestIntersection(other->WorldBound);
  bool possiblyCollidesWith(SVSObject* other);

private:
  SVSObject* parent;
  vector< SVSObject* > children;
  
  // Note that this is a Wm3 smart pointer. These are used internally in Wm3,
  // so we need them here so that system doesn't delete them. This (along with
  // pLTM) are the only places smart ptrs need to be, since that is where the
  // pointers are owned.
  SpatialPtr wm3Unit;
  string id;
  string classId;
  string name;
  bool imagined;
  bool primitive;
  int inhibitedLevel;

  // GroundedObject pointers are stored (and owned) here. Since subgoals can add
  // structure to the ends of the scene tree, the GroundedObjects can be
  // different based on what goal level they are retrieved from.
  //
  // This map associates a goal level with a set of groundedObjects. The level
  // is the lowest level (closest to the top) for which the objects are valid.
  // The highest level for which they are valid is one less than the next highest 
  // int in the map.
  //
  // When a new object is added, the scene graph must recursively, for every
  // object above to the root, add a new (empty) set of groundedObjects
  // associated with the level the object is added at, if there isn't already
  // such a set. When an object is removed, the opposite happens (the sets are
  // deleted).
  map< int, GroundedObjectSet > groundedObjects;



  // this must be called whenever the object structurally changes
  // (a decendant is added or removed)
  void invalidateGroundedObjects(); 
  
  // Move the object so the global transformation to its centroid is this.
  // Local transformation will be calculated accordingly.
  // This is the main way of moving objects.
  // This should only be called by SpatialScene, who is a friend,
  // since the scene needs to determine the change set of everything below this
  // node and tell other modules about it
  void setGlobalTransformation(Transformation tx);


};
