  class SpatialScene {

  public:
  
    SpatialScene();
    ~SpatialScene();

    // main interface for changing objects
    
    // Add a new object to the tree bottom of the tree below parent.
    // The local transformation on the new object should become its global
    // transformation, addObject() will take care of deriving a new local.
    //
    // The object passed in may have substructure, addObject() must parse it
    // all so all subobjects are on the added list.
    bool addObject(string parent, SVSObject* object);
    
    // Add a new object, possibly with substructure, to the middle of the tree, 
    // above the child given.
    // Again, the local tx on the new object will become its global, this
    // function will take care of adjusting the local on it and on its new
    // child (so the new child stays stationary).
    //
    // This is only allowed at top state! It is a nonmonotonic change, since
    // the child (and all its decendents) will be effected. To allow this at
    // lower states, lots of meta information would be needed to encode what
    // each substate thinks the graph looks like, instead of just pruning off
    // the ends of the tree at higher states.
    bool addObjectInterior(string child, SVSObject* object);
    
    bool removeObject(string rootName);

    // This undoes an addObjectInterior, it reconnects the childToKeep to
    // rootName->parent.
    bool removeObjectInterior(string rootName, string childToKeep);

    // This is the interface for moving objects, either a new local or global
    // transformation can be provided. Additionally, if positionOnly=true, the
    // rotation and scaling of the existing object will be retained.
    // (Need to determine if all this is needed, but it shouldn't be too hard
    // to implement)
    bool updateObjectTransformation(string objectId, Transformation newTx, 
        bool positionOnly=false, bool isGlobal=true);


    // Used to get a reference to an object in the scene. This is the main way
    // the scene is read.
    // Returns null if id isn't present.
    // Caller does NOT own the pointer.
    // Pointer is only guaranteed valid for the current decision
    SVSObject* retrieve(string id);

    // The scene has to handle these (rather than the SVSObjects themselves) so
    // it can maintain its list of active ids
    bool expand(string rootName, bool allTheWay);
    bool collapse(string rootName);

    // interface to SoarIO to report Scene contents
    vector< SVSObject* > getAddObjects();
    vector< SVSObject* > getChangeObjects();

    // remove needs the id of the removed node, since the ptr is no longer
    // valid by the point it is read
    set< string > getRemoveObjects();

    // interface to re-evaluate queries. This is different from the
    // above getChangeNodes():
    // -This is set recursive to the leaves, since changes above can impact the
    // coordinates of things below. Changes reported to the contents link only
    // need to cover the root object changed.
    // -This is cleared delibrately, since all queries need to get all the changes
    // (ChangeObjects are cleared when read)
    // This is also used for error reporting: if a command references an object that
    // is missing, that can be OK if it just changed (implying it was
    // removed this decision, which the agent hasn't had a chance to react to
    // yet)
    bool isObjectChangedThisDecision(string object);
    // this is called at the end of every decision to clear the changed set
    // accessed above
    void nextDecision();

  private:
  };

