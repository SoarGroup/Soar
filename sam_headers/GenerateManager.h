
// This singleton class handles connecting generate command watchers to the
// functions that actually do the geometry processing
//
// This should probably all be static.

// This helps insulate qualitative/SVSObject representations in the system from
// geometric representations.

//
//  Retrieve commands are handled specially, since they need to clone tree
//  structure rather than make a new object based solely on geometric info. For
//  those, the SVSObject* is looked up in the spatial scene or pLTM and
//  SVSObject::clone is called on it to get a new set of nodes. The
//  manipulateObject function is called in the root of the result as if
//  the object came from generateObject.

class GenerateManager {
public:
  // constructor goes here (initialize the map of types to generator functions)

  // Return a new SVSObject of the generated object, which the caller will now
  // own (note that it isn't in the spatial scene yet, the generate watcher
  // does that).
  //
  // This is intended for direct predicate projection.
  SVSObject* generateObject(string type, 
      vector< Parameter > parameters);
  
  // Manipulate the primaryObject such that it follows the
  // qualitative specification.
  //
  // This is intended to be used for indirect predicate projection. The
  // manipulation function will return a Transformation indicating 
  // the global transformation of the object at its new location. manipulateObject() 
  // must then change the transformation on the primaryObject (which is, at
  // this point, rooted at the scene origin) by applying that tx to the primaryObject
  //
  bool manipulateObject(string type,
      SVSObject* primaryObject,
      string primaryObjectInterpretation,
      vector< Parameter > parameters);
private:
  // table of types to generators and manipulators goes here
  
  // Return an object from STM or LTM (clone the scene tree structure)
  // This is called by generateObject if the type is "retrieve", rather than
  // deferring to an object generator function.
  SVSObject* retrieveObject(string id, bool isLTM);
  

};

// example object generator header
// these all must have the same prototype, there will be a switch statement
// inside GenerateManager that will associate type strings to function
// ptrs

GroundedObject* generateConvexHull(vector< Parameter > parameters); 
                                  // should contain one or more object parameters
                                  // similar to extraction functions,
                                  // parameters.ground() can be immediately
                                  // called if the function only deals with
                                  // geometry.

// example manipulation header
Transformation applyAnalogicalTransform(GroundedObject* primaryObject,
                                         vector< Parameter > parameters); 
                                        // should contain SVSObject
                                        // parameters:
                                        // source-primary-object
                                        // source-reference-object
                                        // source-frame-object
                                        // reference-object
                                        // reference-frame-object

