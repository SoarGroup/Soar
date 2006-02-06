package edu.umich.JavaBaseEnvironment;
/* File: SimulationControl.java
 * Aug 10, 2004
 */

import java.io.File;
import java.util.ArrayList;
import java.util.Iterator;

import sml.Agent;

/**
 * Defines the abstract class that the model behind a Soar simulation must extend.
 * @author jduchi
 */
public abstract class SimulationControl {

	private static final int MapHeight = 17;
	private static final int MapWidth = 17;
	protected static final int NOTHING = -34567;
	
	/** Parameter to identify whether the simulation has been quit, so late notifications
	 * are not sent. */
	protected boolean quit = false;
	protected int myWorldCount = 0;
	protected boolean running = false;
	protected boolean singleStep = false;
	
	protected ArrayList myAgents = new ArrayList();
	
	protected AgentColors myEcs = new AgentColors();
	protected ArrayList simulationControlListeners = null;
	protected Object[][] myMap = new Object[MapWidth][MapHeight];
	
	/**
	 * Directory for maps
	 */
	protected String mapPath = null;

	/**
	 * Directory for maps
	 */
	protected String agentPath = null;
	
	/**
	 * Accessor for map directory
	 */
	public String getMapPath() {
		return mapPath;
	}

	/**
	 * Accessor for agent directory
	 */
	public String getAgentPath() {
		return agentPath;
	}

	/**
	 * Method defined to, if the simulation is not running, start the simulation.
	 */
	public abstract void runSimulation();

	/**
	 * Prints out the map being used internally by a SimulationControl.
	 */
	public abstract void printMap();
	
	/**
	 * Returns the height of the map being used internally by the Simulation.
	 * If subclass is changing the width of the map being used, make sure to
	 * implement this function.
	 */
	public int getMapHeight(){
		return(MapHeight);
	}
	
	/**
	 * Reload all of an agent's productions without an excise
	 * @param color color of agent to operate on
	 */
	public void reload(String color)
	{
		Agent agent = getAgentByColor(color);
		SoarAgent soarAgent = getSoarAgentByColor(color);
		agent.LoadProductions(soarAgent.getProductionPath());
	}
	
	/**
	 * Excises all of an agent's productions
	 * @param color color of agent to operate on
	 */
	public void exciseAll(String color)
	{
		getAgentByColor(color).ExecuteCommandLine("excise -a");
	}
	
	/**
	 * Excises all of an agent's chunks
	 * @param color color of agent to operate on
	 */
	public void exciseChunks(String color)
	{
		getAgentByColor(color).ExecuteCommandLine("excise -c");
	}
	
	/**
	 * @param color color of agent to get
	 * @return Agent object
	 */
	public abstract Agent getAgentByColor(String color);
	
	/**
	 * @param color color of agent to get
	 * @return Agent object
	 */
	public abstract SoarAgent getSoarAgentByColor(String color);
	
	/**
	 * Returns the width of the map being used internally by the Simulation.
	 * If subclass is changing the width of the map being used, make sure to
	 * implement this function.
	 */
	public int getMapWidth(){
		return(MapWidth);
	}
	
	/**
	 * Tells whether the simulation is currently running.
	 * @return <code>true</code> if the simulation is running, <code>false</code> otherwise.
	 */
	public boolean isRunning(){ return (running); }
	
	/**
	 * Tells whether the simulation is currently in the middle of a step.
	 * @return <code>true</code> if the simulation is in a step, <code>false</code> otherwise.
	 */
	public boolean isStepping(){ return (singleStep); }
	
	/**
	 * If the simulation is running, will stop the simulation after all the agents
	 * have finished the current step.
	 * Implementation is simply as follows:<p>
	 * <code>running = false; singleStep = false; fireSimEndedNotification();</code>
	 */
	public void stopSimulation(){
		running = false;
		singleStep = false;
		fireSimEndedNotification();
	}
	/**
	 * Implementers should, if simulation is running, stop the simulation after all
	 * agents have finished current step, and then will notify all listeners that the
	 * simulation has been quit and stop execution.
	 */
	public void quitSimulation(){
		quit = true;
		stopSimulation();
		fireQuitNotification();
	}
	
	/**
	 * Tells caller whether the simulation has quit.
	 * @return <code>true</code> if the simulation has quit, <code>false</code> otherwise.
	 */
	public boolean hasQuit(){
		return(quit);
	}
	
	/**
	 * Method to notify caller whether simulation should stop.
	 * @return <code>true</code> if the simulation should stop, false otherwise.
	 */
	public abstract boolean simShouldStop();
	
	/**
	 * Causes the simulation to take a single step, allowing each agent
	 * to evaluate one decision (or movement, depending on the wait until
	 * decision/movement flag).
	 * <p>May cause undefined operation if an agent is destroyed
	 * in the middle of a step.
	 */
	public void singleStep()
	{
		singleStep = true;
		if(simShouldStop()){
			stopSimulation();
			singleStep = false;
			running = false;
			return;
		}
		Object decisions[] = new Object[myAgents.size()];
		Iterator iter = myAgents.iterator();
		int count = 0;
		while(iter.hasNext()){
			SoarAgent curr = ((SoarAgent)iter.next());
			decisions[count++] = curr.makeDecision();
		}
		iter = myAgents.iterator();
		count = 0;
		while(iter.hasNext()){
			evaluateDecision(decisions[count++], (SoarAgent)iter.next());
		}
		++myWorldCount;
		fireWorldCountChangedNotification();
		singleStep = false;
	}
	
	/**
	 * Determines what actions this SimulationControl needs to take based on a given
	 * <code>SoarAgent</code>'s decision.
	 * @param decision An <code>Object</code> corresponding to the decision taken by the 
	 * <code>SoarAgent</code>, as defined in the class implementing <code>SoarAgent</code>
	 * (or elsewhere).
	 * @param sa The <code>SoarAgent</code> whose decision we are evaluating.
	 */
	protected abstract void evaluateDecision(Object decision, SoarAgent sa);
	
	/**
	 * Prints to the standard output the status of all the Agents in the simulation
	 * as defined by the implementer of <code>SimulationControl</code>.
	 */
	public abstract void printAgentStats();
	
	/**
	 * Creates a new SoarAgent that is human-controlled with the specified color.
	 * <p>Some implementations may choose not to have a human-controlled agent.</p>
	 * @param color The String name of the color used by the newly created
	 * SoarAgent.
	 * @return The newly created SoarAgent, or null if none was created.
	 */
	public abstract SoarAgent createHumanAgent(String color);
	

	/**
	 * Resets the map to some sort of original state.
	 */
	public abstract void resetMap();
	
	/**
	 * Generates a new random map for the simulation. Implementers should have
	 * any existant agents randomly placed on the map. Notifies listeners of
	 * the change in the map.
	 */
	public abstract void newRandomMap();
	
	/**
	 * Loads the file specified to be the SimulationControl's internal map. The file should
	 * determine a MapWidth * MapHeight map, with entries defined by the integers
	 * in SimulationControl for elements of environment.
	 * @param infile The file to be loaded as the map. Can be any file with correct formatting.
	 */
	public abstract void loadMap(File infile);
	
	/**
	 * When this method is called, implementing classes should check <code>infile</code>
	 * to see if it is a Soar file or a simulation-specific file. Then they should
	 * take the appropriate action, attaching source code if <code>infile</code> is a
	 * Soar file, or doing other simulation-specific actions if it is not (for example,
	 * it might be a *.stank file, specifying parameters for a <code>Tank</code>.
	 * @param infile The <code>File</code> to be loaded.
	 * @param colorName The <code>String</code> name of the color the agent should have,
	 * or <code>null</code> if it does not matter.
	 * @return The <code>SoarAgent</code> that has been created.
	 */
	public abstract SoarAgent loadAgent(File infile, String colorName);
	
	/**
	 * Gives the caller an array of the names of colors that are not in
	 * use by any of the current agents in the simulation.
	 * @return An array of Strings naming the colors available for agents.
	 */
	public String[] getColorsAvailable(){
		return(myEcs.colorsAvailable());
	}
	
	/**
	 * Gives the caller an array of the names of colors that are in use
	 * by the current agents in the simulation.
	 * @return An array of Strings naming the colors being used.
	 */
	public String[] getColorsUsed(){
		return(myEcs.colorsUsed());
	}
	
	/**
	 * Returns whether any agents can be destroyed.
	 * Implementers should use the following strategy:
	 * @return <code>true</code> if the simulation is not running or in the middle of a step, allowing
	 * agents to be destroyed, false otherwise.
	 */
	public boolean canDestroyAgent(){
		return(!running && !singleStep);
	}	
	/**
	 * Destroy all the agents in the simulation.
	 */
	public void destroyAllAgents()
  {
		if(running || singleStep) return;
		Iterator iter = myAgents.iterator();
		while(iter.hasNext())
		{
			removeAgentFromSimulation((SoarAgent)iter.next());
		}
		myAgents.clear();
	}
	
	/**
	 * Removes the specified agent from the simulation permanently, returning its
	 * color to the simulation.
	 * @param destroyed The <code>SoarAgent</code> to permanently remove from the simulation.
	 */
	protected abstract void removeAgentFromSimulation(SoarAgent destroyed);
	
	/**
	 * Destroys the agent with the color named by colorName. Agent stops being updated
	 * and listeners are notified that agent has been destroyed.
	 * <p><p>If this method is invoked while the simulation is running or while a step
	 * is being taken, it simply returns, because the simulation may be waiting on
	 * the given Eater.</p>
	 * @param colorName The String name of the color of the agent to be destroyed.
	 * @return The destroyed SoarAgent if destruction was successful, null otherwise.
	 */
	public SoarAgent destroyAgent(String colorName){
		if(running || singleStep) return null;
		Iterator iter = myAgents.iterator();
		SoarAgent e = null;
		while(iter.hasNext()){
			e = (SoarAgent)iter.next();
			if(e.getColorName().equals(colorName)) break;
		}
		if(e != null){
			iter.remove();
			removeAgentFromSimulation(e);
		}
		return (e);
	}

	/**
	 * Destroys the given SoarAgent, as defined by SoarEater's equals() method.
	 * Nothing happens if the given SoarAgent is not in this SimulationControl.
	 * <p><p>If this method is invoked while the simulation is running or while a step
	 * is being taken, it simply returns, because the simulation may be waiting
	 * on the given SoarAgent.</p>
	 * @param destroyed The SoarAgent to be destroyed.
	 * @return The destroyed SoarAgent if destruction was successful, null otherwise.
	 */
	public SoarAgent destroyAgent(SoarAgent destroyed){
		if(running || singleStep) return null;
		Iterator iter = myAgents.iterator();
		SoarAgent e = null;
		while(iter.hasNext()){
			e = (SoarAgent)iter.next();
			if(e.equals(destroyed)) break;
			e = null;
		}
		if(e != null){
			iter.remove();
			removeAgentFromSimulation(destroyed);
		}
		return (e);
	}

	/**
	 * Gives the caller an array of the <code>SoarAgent</code>s in the simulation.
	 * Does not directly affect how they are stored, so modification of this
	 * array is allowed, though modification of <code>SoarAgent</code>s themselves should be avoided.
	 * @return An array of all the <code>SoarAgent</code>s in the simulation, as defined by
	 * method <code>toArray()</code> in <code>ArrayList</code>.
	 * @see java.util.ArrayList#toArray(java.lang.Object[])
	 */
	public SoarAgent[] getAllAgents(){
		return((SoarAgent[])myAgents.toArray(new SoarAgent[0]));
	}
	
	/**
	 * Resets all the SoarAgents in the simulation as determined by the agent's
	 * reset() method.
	 */
	protected void resetAgents(){
		Iterator iter = myAgents.iterator();
		while(iter.hasNext()){
			((SoarAgent)iter.next()).reset();
		}
	}
	
	/**
	 * Gives the caller the current world count of the simulation, which is incremented
	 * at every step.
	 * @return The current world count of the simulation.
	 */
	public int getWorldCount(){
		return(myWorldCount);
	}
	
	/**
	 * Convenience method to get the <code>SimulationControl</code> to sleep for some number of
	 * milliseconds.
	 * @param milliseconds The number of milliseconds to have <code>SimulationControl</code> sleep.
	 */
	public void waitATouch(int milliseconds){
		if(milliseconds < 0) return;
		try{
			Thread.sleep(milliseconds);
		} catch (InterruptedException ignored){}
	}
	
	/**
	 * Method used to find the contents of the specified location in the simulation's internal map.
	 * @param loc The location being queried.
	 * @return An element of the Simulation. If the x or y
	 * coordinate is outside the indices of the map, returns a TSWall.
	 */
	public Object getLocationContents(Location loc){
		return(getLocationContents(loc.getX(), loc.getY()));
	}
	
	/**
	 * Method used to find the contents of the specified position in the simulation's internal map.
	 * @param x The x-coordinate of the location being queried.
	 * @param y The y-coordinate of the location being queried.
	 * @return An element of the Simulation. If the x or y
	 * coordinate is outside the indices of the map, returns a <code>Wall</code>.
	 */
	public abstract Object getLocationContents(int x, int y);
	
	/*---------------LISTENER BUSINESS---------------*/
	
	/**
	 * Adds the specified SimulationControlListener to the listeners to this instance
	 * of SimulationControl.
	 * @param scl The SimulationControlListener to be added.
	 */
	public void addSimulationControlListener(SimulationControlListener scl){
		if(simulationControlListeners == null) simulationControlListeners = new ArrayList();
		controlListenersToAdd.add(scl);
	}
	/**
	 * An <code>ArrayList</code> containing <code>SimulationControlListener</code>s
	 * to remove from the listeners to this <code>SimulationControl</code>. Acts
	 * as a cache.
	 */
	protected ArrayList controlListenersToRemove = new ArrayList();
	/**
	 * An <code>ArrayList</code> containing <code>SimulationControlListener</code>s
	 * to add to the listeners to this <code>SimulationControl</code>. Acts
	 * as a cache.
	 */
	protected ArrayList controlListenersToAdd = new ArrayList();
	/**
	 * Removes the specified <code>SimulationControlListener</code> from the listeners
	 * to this instance of SimulationControl by placing it in an <code>ArrayList</code>
	 * of listeners to be removed (acts as a sort of cache of them).
	 * @param scl The <code>SimulationControlListener</code> to be removed from the listeners.
	 */
	public void removeSimulationControlListener(SimulationControlListener scl){
		if(simulationControlListeners == null) return;
		controlListenersToRemove.add(scl);
	}
	
	/**
	 * Called before the firing of any notification. Checks to see whether
	 * any <code>SimulationControlListener</code>s have requested removal from
	 * notifications or whether any have requested to be added to the listeners.
	 */
	protected void checkAddRemovals(){
		Iterator iter = controlListenersToRemove.iterator();
		while(iter.hasNext()){
			if(simulationControlListeners == null) break;
			simulationControlListeners.remove(iter.next());
		}
		controlListenersToRemove.clear();
		iter = controlListenersToAdd.iterator();
		while(iter.hasNext()){
			simulationControlListeners.add(iter.next());
		}
		controlListenersToAdd.clear();
	}

	/**
	 * Notifies listeners that an eater has been created.
	 * @param created The eater created.
	 */
	protected void fireAgentCreatedNotification(SoarAgent created){
		checkAddRemovals();
		if(simulationControlListeners == null) return;
		Iterator iter = simulationControlListeners.iterator();
		while(iter.hasNext()){
			((SimulationControlListener)iter.next()).agentCreated(created);
		}
	}
	
	/**
	 * Notifies listeners that an eater has been destroyed.
	 * @param destroyed A convenience pointer to the eater destroyed.
	 */
	protected void fireAgentDestroyedNotification(SoarAgent destroyed){
		checkAddRemovals();
		if(simulationControlListeners == null) return;
		Iterator iter = simulationControlListeners.iterator();
		while(iter.hasNext()){
			((SimulationControlListener)iter.next()).agentDestroyed(destroyed);
		}
	}
	
	/**
	 * Notifies listeners that the simulation has been started running.
	 */
	protected void fireSimStartedNotification(){
		checkAddRemovals();
		if(simulationControlListeners == null) return;
		Iterator iter = simulationControlListeners.iterator();
		while(iter.hasNext()){
			((SimulationControlListener)iter.next()).simStarted();
		}
	}
	
	/**
	 * Notifies listeners that the simulation has been stopped.
	 */	
	protected void fireSimEndedNotification(){
		fireSimEndedNotification(null);
	}
	
	protected void fireSimEndedNotification(String message){
		checkAddRemovals();
		if(simulationControlListeners == null) return;
		Iterator iter = simulationControlListeners.iterator();
		while(iter.hasNext()){
			((SimulationControlListener)iter.next()).simEnded(message);
		}
	}
	
	/**
	 * Notifies listeners that the world count of the simulation has changed.
	 * Also fires the <code>fireFinishedWorldCountUpdating</code> method when
	 * it is finished.
	 */
	protected void fireWorldCountChangedNotification(){
		checkAddRemovals();
		if(simulationControlListeners == null) return;
		Iterator iter = simulationControlListeners.iterator();
		while(iter.hasNext()){
			((SimulationControlListener)iter.next()).worldCountChanged(myWorldCount);
		}
		fireFinishedWorldCountUpdating();
	}
	
	/**
	 * Notifies listeners that the location specified has changed.
	 */
	public void fireLocationChangedNotification(Location changed){
		checkAddRemovals();
		if(simulationControlListeners == null) return;
		Iterator iter = simulationControlListeners.iterator();
		while(iter.hasNext()){
			((SimulationControlListener)iter.next()).locationChanged(changed);
		}
	}
	
	protected void fireQuitNotification(){
		checkAddRemovals();
		if(simulationControlListeners == null) return;
		Iterator iter = simulationControlListeners.iterator();
		while(iter.hasNext()){
			((SimulationControlListener)iter.next()).simQuit();
		}
	}
	
	/**
	 * Notifies all listeners that a new map has been created. If there is a message to go
	 * along with the new map, for instance, that its randomness is deterministic (as
	 * specified in <code>TankSoarJControl</code>), this message is passed as well.
	 * @param message The <code>String</code> message being sent, if any.
	 */
	protected void fireNewMapNotification(String message){
		checkAddRemovals();
		Iterator iter = myAgents.iterator();
		while(iter.hasNext()){
			((SoarAgent)iter.next()).reset();
		}
		if(simulationControlListeners == null) return;
		iter = simulationControlListeners.iterator();
		while(iter.hasNext()){
			((SimulationControlListener)iter.next()).newMap(message);
		}
	}

	/** An <code>ArrayList</code> containing the listeners to be notified when
	 * the simulation finishes notifying listeners that the world count has
	 * changed. */
	private ArrayList worldCountNotificationListeners = null;
	
	/**
	 * Fires notification for all <code>WorldCountNotificationListener</code>s
	 * that the world count has been changed and all <code>SimulationControlListener</code>s
	 * have been notified that it has changed.
	 */
	protected void fireFinishedWorldCountUpdating(){
		if(worldCountNotificationListeners == null) return;
		Iterator iter = worldCountNotificationListeners.iterator();
		while(iter.hasNext()){
			((WorldCountNotificationListener)iter.next()).finishedWorldCountUpdating();
		}
	}
	
	/**
	 * Adds the specified <code>WorldCountNotificationListener</code> to the set
	 * of listeners notified when world count has been changed and all <code>
	 * SimulationControlListener</code>s have been notified that it has changed.
	 * @param toAdd The <code>WorldCountNotificationListener</code> to add.
	 */
	public void addWorldCountNotificationListener(WorldCountNotificationListener toAdd){
		if(worldCountNotificationListeners == null) worldCountNotificationListeners = new ArrayList();
		worldCountNotificationListeners.add(toAdd);
	}

	/**
	 * Removes the specified <code>WorldCountNotificationListener</code> from the set
	 * of listeners notified when world count has been changed and all <code>
	 * SimulationControlListener</code>s have been notified that it has changed.
	 * @param toRemove The <code>WorldCountNotificationListener</code> to add.
	 */
	public void removeWorldCountNotificationListener(WorldCountNotificationListener toRemove){
		if(worldCountNotificationListeners == null) return;
		worldCountNotificationListeners.remove(toRemove);
	}
	
	public boolean isSteppable() {
		return true;
	}

	public boolean isRunnable() {
		return true;
	}

	public boolean isStoppable() {
		return true;
	}

	public boolean isQuittable() {
		return true;
	}
}
