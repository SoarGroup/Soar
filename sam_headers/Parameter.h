
// base class for parameters
class Parameter {
public:
  Parameter();
  enum Type {
    int,
    string,
    float,
    SVSObject,
    GroundedObject
  };
  
  virtual Type getType() = 0;

  // each subclass implements all of these,
  // return "this" if the type matches, null otherwise
  virtual IntParameter* pointerAsIntType() = 0;
  virtual StringParameter* pointerAsStringType() = 0;
  virtual FloatParameter* pointerAsFloatType() = 0;
  virtual SVSObjectParameter* pointerAsSVSObjectType() = 0;
  virtual GroundedObjectParameter* pointerAsGroundedObjectType() = 0;

  // ALTERNATE VERSION
  // each subclass implements all of these,
  // return "this" if the type matches, null otherwise
  virtual int* intValue() = 0;
  virtual string* stringValue() = 0;
  virtual float* floatValue() = 0;
  virtual SVSObject* SVSObjectValue() = 0;
  virtual GroundedObject* GroundedObjectValue() = 0;

};

class GroundedObjectParameter: public Parameter {
public:
  GroundedObjectParameter(string _name, GroundedObject* _val);
  string name;
  GroundedObject* value;
};

class SVSObjectParameter: public Parameter {
public:
  SVSObjectParameter(string _name, SVSObject* _val);
  string name;
  SVSObject* value;
};

class IntParameter: public Parameter {
public:
  IntParameter(string _name, int _val);
  string name;
  int value;
};

// etc. for Identifiers, strings, floats, and bools
// need to determine whether to use dynamic_cast, or have an enum to detect
// types

// Instances of this class are passed to most primitive SVS function
class ParameterSet {
public: 
  // each parameter is added one-by-one (e.g., during parsing)
  void add(Parameter p);

  // get() provides an associative interface to the set
  // the return value is the number of parameters in the set with the given
  // name (e.g., 0 means not found)
  //
  // out_param is assigned to a relevant parameter
  // if there are multiple, the assignment of out_param is arbitrary
  int get(string name, Parameter& out_param);

  // iteration interface to the set, calling getFirst resets the iteration
  Parameter getFirst();
  bool getNext(Parameter& out_param); // false if no more

  // remove parameter(s) with the given name from the set
  // returns the number of removals
  int remove(string name);
  
  // This function goes through all parameters and converts all identifier
  // parameters to SVSObject* parameters
  //
  // If allowLTM==true, identifier parameters ending in -class are
  // looked up in LTM, and the -class is removed from the name. If
  // allowLTM==false and such a WME is discovered, return false as this is an
  // error (e.g., LTM references are not allowed in extraction)
  bool lookupObjects(bool allowLTMAccess);

  // this function goes through all parameters and grounds the objects
  //
  // This is done as follows:
  // for all SVSObject* parameters ending in (something)-object:
  //  look for another parameter (something)-object-interpretation
  //    if found, get the GroundedObject of that interpretation, and remove the
  //      interpretation parameter
  //    otherwise, get the poly3d GroundedObject
  //  convert the identifier parameter to be a GroundedObject parameter
  //  remove the interpretation parameter from the set
  //
  //  return false if some object isn't found, and print an error if something
  //  isn't found and the agent should have known better (it didn't just
  //  disappear this decision)
  bool groundObjects();

  // In general, command watchers will build parameter sets that have Identifiers for
  // objects. These might be internally cached and reused, even as the
  // SVSObject*s that would result from a lookup might change.
  // Before passing to managers, the watcher will call
  // lookupObjects(). Once the ParameterSet gets down to a primitive process,
  // that process will call groundObjects() if necessary.

private:
  // the names are mapped to the parameters
  multimap< string, Parameter > contents;
};


