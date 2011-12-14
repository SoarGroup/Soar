
// Motion models are patterns of movement attached to objects in the scene
// Theoretically, they should be recognized, but can currently be
// deliberately attached
//
// Motion models have types, in LTM they exist as categories
// A motion model instantiated in STM (through a track-motion command)
// learns parameters based on how the object it is attached to moves
// (e.g., learns that allocentric translation for object X means 2units/dec.
// north)
//
// During tracking, the agent can specify whether the tracking process should
// propogate changes to LTM (if the model supports it), or only locally track
//
// A motion model can spawn a simulator, which is the forward-projection
// version of itself. They can attach to different objects, so they are
// different things.
//
// Motion models can be present that ignore all tracking (e.g., old SRS-type
// motions).
// 
// A simulation can be invoked based on a model in LTM or STM. 
//
// In addition, through a store-motion command, the agent can add new LTM
// motion models. These copy an existing LTM model (and its learned state).
// This may not make sense for non-learning models, but is syntactically valid.
//
// From the POV of the motion model, this distinction is unimportant, however.

class Simulation {
public:
  // The parameters passed to the constructor are model-specific internal
  // parameters. These are the things that have been learned.
  Simulation(ParameterSet parameters);

  // Simulations should not move existing objects in the scene, but should
  // rather clone them, add the clone to the scene, and manipulate that.
  // These cloned objects should be returned from the init function, and
  // will appear on the command.result structure. Parameters here are those
  // parsed from WM. 
  virtual vector< SVSObject* > init(ParameterSet parameters, double time) = 0;

  // This process updates the spatial scene (via the appropriate interface) in
  // arbitrary ways, given some constraints:
  // - only objects created by the simulation should be manipulated
  // - all objects created must be cleaned up (removed from the scene) in the
  // destructor
  // - at least one new object must be returned by the init function. More can
  // be present, but the agent will have to access them via
  // spatial-scene.contents
  virtual bool update(ParameterSet parameters, double time) = 0;

private:
  // arbitrary internal state is allowed here (but remember not to keep
  // SVSObject*'s from the parameters)
};

class MotionModel {
public:
  MotionModel(bool _isTemporary) {
    temporary = _isTemporary;
  }
  ~MotionModel(); 

  // this is the interface new motions must implement:
  
  // startSession() tells the model to start tracking the provided object using
  // the given (model-specific) parameters
  // The current time (in terms of an environmental event counter) is provided.
  virtual bool startSession(ParameterSet parameters,
                            double time) = 0;
  
  // Learn is called every time a new datapoint (may) available. The model is
  // responsible for taking into account the time difference since the
  // startSession() call or the last learn() call and handling it
  // appropriately. This difference may be zero,
  // if the agent is ignoring the environment.
  virtual bool learn(vector< Parameter> parameters, double time) = 0;
  
  // Some models may require at least one datapoint before doing anything. This
  // queries the model to determine if a simulation is yet possible.
  virtual bool isReady() = 0;

  // Spawn a Simulation object (owned by the caller) instantiating the
  // motion. The MotionModel derived class will construct the learned
  // parameters and pass them to the Simulation constructor.
  // 
  // The caller is then responsible for calling Simulation::init() and update.
  virtual Simulation* createNewSimulation() = 0;


  // below here is handled by the base class, subclasses need not be concerned
  
  // temporary is true if the motion is in STM only, and state will be forgotten once
  // tracking ceases
  bool isSTMOnly() {
    return STMOnly;
  }

  // a given motion model can only handle one learning session at once,
  // MotionModelManager uses this to ensure only one is started
  bool isTracking() {
    return tracking;
  }


  void setId(string _id) {
    id = _id;
  }
  string getId() {
    return id;
  }
protected:
  bool STMOnly;
  string id;

};


// MotionControllers are a subset of motion models, that spawn
// ControllerInstances, which are a derived class of Simulation
//
// TODO: determine what a MotionSimulation should return, and how to get it to
// the environment

class ControllerInstance : public MotionSimulation {
  virtual void  initControl(ParameterSet parameters, double time) = 0;
  virtual vector< double > updateControl(ParameterSet parameters, double time) = 0;
};


class MotionController : public MotionModel {
  public:
    virtual ControllerInstance* createNewControllerInstance() = 0;
};

// The motion model interface above is much more general than that used for
// extraction/projection. The SimpleSimulation and SimpleMotionModel classes are
// more constrained, but provide an easier interface at the same level as
// extraction/generation.
//
// - a SimpleMotionModel captures the movement of exactly one object
// - SimpleMotionModel base class clones the moving object when a simulation starts,
// and passes it to the simulation implementation
// - implementation code sees everything in terms of GroundedObjects
// - SimpleMotionModels do not support simulations/tracking processes where the
// set of objects changes, so the implementation can keep internal state that
// has GroundedObject*s in it. These might move through space, but the ptrs
// will be guaranteed valid.
//
// Something similar could be done for controllers.
//
// This might be commented out for the initial release, but I think having a
// clean, simple interface here would be very useful to others making their own
// motion models, so they don't have to know any parts of the system beyond
// geometry.
class SimpleSimulation : public MotionSimulation {
public:
  // SimpleSimulation init:
  // - Clone the (require) moving object from the parameter list, and remove it
  // from the list
  // - Ground all objects
  // - Call the derived class init function, passing in the grounded, cloned
  // moving object separately from the parameter vector
  // - Return a vector holding only the moving object
  vector< SVSObject* >  init(ParameterSet parameters, double time);

  // Take the parameters, and ground all objects. Ensure that the objects at
  // each update are the same objects that were passed to the constructor, otherwise
  // fail. Call updateSimpleSimulation with the time, it will change the GroundedObject 
  // for the moving object. Manipulate the spatial scene accordingly.
  bool update(ParameterSet parameters, double time);

  // Derived class init function sees all grounded parameters, and does
  // initialization processing (if any). It must store the moving GroundedObject
  // and any parameters internally, as they won't be seen again.
  virtual bool init(GroundedObject* movingObject, ParameterSet parameters) = 0;

  // Derived class update just sees a time step. It manipulates the position of the
  // movingObject accordingly.
  virtual bool update(double time) = 0;
private:
};

class SimpleMotionModel : public MotionModel {
  // startSession() for this class will ground all parameters and call the
  // derived class startSession() (below) that takes grounded parameters and
  // specifies the movingObject. Similar to SimpleSimulation, this class needs
  // to monitor all objects it gets over updates, so it can ensure the
  // GroundedObject*s it passes during startSession remain valid (erroring out
  // otherwise).
  bool startSession(ParameterSet parameters, double time) = 0;
  
  // Derived class must implement this, from its perspective, all objects are
  // grounded, and all ptrs passed at init are guaranteed valid (although the
  // objects themselves may change).
  virtual bool startSession(GroundedObject* movingObject, 
      ParameterSet parameters, double time) = 0;

  // SimpleMotionModel learn function checks that the objects are the same as
  // passed to init and grounds them, also confirming that the GroundedObject*s
  // don't change from time to time.
  bool learn(vector< Parameter> parameters, double time);
  
  // Derived class learn function just sees a time, it should have cached the
  // GroundedObject*s passed to it in init, and they are guaranteed still
  // valid.
  virtual bool learn(double time) = 0;

};
