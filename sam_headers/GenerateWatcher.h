
class GenerateWatcher : public CommandWatcher {
public: 

  // constructor stores the commandWME, and calls CommandWatcher constructor
  // specifying Early phase 
  GenerateWatcher(Identifier* _commandWME, int _time);


  // All of the action occurs during the first call to updateResult().
  // Subsequent calls check if anything in the command changed, and remove the
  // image and add status error if that is the case: dynamic imagery is
  // unsupported (outside of the case where an image moves due to
  // manipulations above it in the scene tree).
  //
  // There is some recursion necessary, so to parse the image, the function
  // buildObject() is called, passing in the root command WME. After this
  // returns, there will be an SVSObject*, possibly with substructure. This
  // must then be added to the spatial scene. The top-level command has some
  // parameters that specify where in the scene tree to add the whole structure
  // (child-object or parent-object). If these are present, updateResult()
  // needs to convert them to string ids and pass them to the appropriate
  // SpatialScene function (addObject or addObjectInterior).
  //
  // Once all is complete, the result structure needs to be built. Probably the
  // best way to do this is to only add a result.object identifier (using the
  // appropriate perceptual pointer identifier) referring to the root new
  // object, and letting the agent access any substructure via
  // spatial-scene.contents.
  void updateResult();

  // The destructor must remove all created objects from the scene, and delete
  // them. If the attachment point was in the interior, the interior removal
  // function must be called.
  ~GenerateWatcher();
private:

  // To build an object, the WME tree starting at commandRoot must be parsed 
  // to determine a type and
  // build a list of parameters based on what is below object-source (if it
  // exists). CommandWatcher::parseParameters() can be used for this. Then,
  // GenerateManager::generateObject is called, getting a new SVSObject*. This
  // object is _not_ yet in the scene, and may have substructure.
  //
  // If an object-source isn't present, buildObject() must make a new grouping
  // SVSObject. In this case (and only in this case), one or more generate-child structures 
  // must be present. For each child, buildObject() should be recursively
  // called, passing in the generate-child identifier as commandRoot and
  // setting isTopLevel false. After each call returns, the SVSObject* should
  // be added as a child of the grouping object created above.
  //
  // At this point, there should be an SVSObject*, and all recursion below the
  // current level (if any) should be done.
  //
  // Then, if a transform-source is present, type and parameters must again be
  // parsed, and GenerateManager::manipulateObject() will be called
  // accordingly.
  //
  // Finally, if a reference-frame-source-object is specified, it should be
  // applied by calling SVSObject::assumeReferenceFrameOf(), and if a name is
  // specified, it should be applied to the SVSObject* through SVSObject::setName().
  // 
  // The resulting SVSObject* is then returned.
  SVSObject* buildObject(Identifier* commandRoot);

  // Note that the recursive process will create a number of SVSObject*s along
  // the way, all of which will be added to the scene if things are successful
  // (although SpatialScene::addObject is only called on the final SVSObject*,
  // all others are also added as the scene parses child links). However, if an
  // error occurs late in parsing, the SVSObject*s generated so far (including
  // all of their children) need to be deleted.
};
