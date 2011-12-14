
class PerceptualLTM {
public:
  // read a set of model files (and textures?) from disc during construction
  //
  // note: geometry of models must be normalized at some point prior to
  // retrieval, so the centroid is the origin of the local FOR
  PerceptualLTM(string directory);

  // retrieve one or more objects from LTM
  // note that this will return a pointer to the root of a tree (assuming the
  // object retrieved isn't primitive)
  // Caller does _not_ own this, but has to call SVSObject.clone() to get a
  // copy.
  //
  // If the retrieved object has a parent in LTM,
  // it is important that the global transformation on it is preserved relative
  // to the root of its tree. In general, that transformation alone is
  // arbitrary and should not be used to position the object, but it has useful
  // meaning relative to other such transformations in LTM for objects in the
  // same scene tree. For example, this is needed to support "place the fork in
  // relation to the plate as the trunk of a car relates to the hood", where
  // "trunk" and "hood" are LTM objects and parts of a common object.
  SVSObject* retrieve(string classId);

 string declareObject(string name, vector< double > xyzList, string textureName = ""); 
};
