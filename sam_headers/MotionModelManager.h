// keeps all of the learned motion models,
// keeps all ongoing simulations

class MotionModelManager {
public:
  MotionModelManager();
  ~MotionModelManager();

  // specialize a long-term motion (copy it)
  string storeMotion(string nameBase, string baseClassId);

  // Start a tracking process, and return the id of the STM model.
  // If storeInLTM is false, the model is copied, and a new STM instance is
  // created, which will be destroyed once tracking stops.
  // If storeInLTM is true, the STM model is a reference to the LTM model, and
  // results will be persistant.
  //
  // Return "" if the id is invalid (does not refer to an LTM motion), or if
  // storeInLTM is true and the model is already tracking something.
  //
  // Time is likely the decision count, but could be an environmental time which may be
  // nonuniform between steps.
  string startTracking(string motionLTId, bool storeInLTM, 
      ParameterSet parameters, double time);
  
  // The tracking, simulation, and control processes will take some parameters, 
  // which will typically include at least an
  // SVSObject* to the moving thing. Note that, for all processes:
  // - the motion model grounds its own objects as needed
  // - parameters may change between steps, this will especially be the case
  // in dynamic worlds where objects may come and go often. A parameter can
  // legally refer to an object that disappears during the current i/o phase.
  // - for that reason, the parameter vector must be reparsed from WM every
  // update, and the MMM must filter out expired objects (and not report
  // errors)
  // - likewise, tracking/simulation/control processes should not store
  // SVSObject*s in their state, but get them fresh from the parameters every
  // step
  // - models aren't limited to tracking/simulating/controlling a single object

  // updateTracking is called every decision by the watcher. Parameters may
  // have changed!
  bool updateTracking(string motionSTId, ParameterSet parameters, double time);
  // Stop the tracking process, and throw out the STM model instantiation, if
  // it is not a reference to an LTM model (ie, storeInLTM == false).
  void stopTracking(string motionSTId);

  // make a new simulation instantiation
  string startSimulation(string motionId, ParameterSet parameters);

  // simulation steps are deliberately controlled by the agent
  // (for tracking and control, agent only says start and stop, and MMM handles
  // updates internally when updateTrackingAndControl() is called)
  // 
  // the simulation step will modify the spatial scene in arbitrary ways as a
  // side effect
  bool stepSimulation(string simulation, ParameterSet parameters, double time);
  bool stopSimulation(string simulationId);

  // make a new control instantiation (controllerId must refer to a MM that is
  // a controller). interface is similar to tracking and simulation
  string startControl(string controllerId, ParameterSet parameters, double time);
  bool updateControl(string controllerId, ParameterSet parameters, double time);
  bool stopControl(string controllerId);

  // interface to SoarIO so the agent has a list of all motion models in STM
  // these are cleared when read
  vector< string > getAddMotionModels();
  vector< string > getRemoveMotionModels();
  vector< string > getReadyMotionModels();

private:
};
