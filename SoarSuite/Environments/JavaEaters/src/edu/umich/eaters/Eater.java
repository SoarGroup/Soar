//* File: Eater.java
// * Jul 2, 2004
// */
//
package edu.umich.eaters;

import edu.umich.JavaBaseEnvironment.SoarAgent;
import edu.umich.JavaBaseEnvironment.Location;

import java.util.*;
import java.io.File;

import javax.swing.JOptionPane;

import sml.*;

/**
 * Each eater is an agent in the Eaters game. This class communicates with Soar
 * and returns to the EaterControl the decisions it has made regarding movement,
 * which EaterControl uses to update itself.
 * @author jduchi
 */
public class Eater implements SoarAgent{	
	private static int agentNumber = 0;
	
	public final String MOVE = "move";
	public final String JUMP = "jump";
	
	public Agent agent = null;;
	private Kernel kernel = null;
	
	public static final int NoDecision = -1;
	public static final int MoveNorth = 0;
	public static final int MoveEast = 1;
	public static final int MoveSouth = 2;
	public static final int MoveWest = 3;
	public static final int JumpNorth = 4;
	public static final int JumpEast = 5;
	public static final int JumpSouth = 6;
	public static final int JumpWest = 7;
		
	protected String myColorName;
	protected int myRecentDecision = NoDecision;
	private int myScore = 0;
	private int myMoveCount = 0;
	
	protected EaterControl myEC;
	protected Location myLocation;
	private ArrayList eListeners;
	private Object[][] lastVisibleSquares;
	private int lastTurnVisibleSquaresCalculated = -1;
	private Location lastLocationVisibleSquaresCalculated = new Location(-1, -1);

	private EaterInputLink iLink;
	
	private String productionPath;
	
	/**
	 * Creates an instance of <code>Eater</code>.
	 * @param loc The location at which this instance of <code>Eater</code> has been created
	 * @param control The <code>EaterControl</code> in which this <code>Eater</code> exists
	 * @param color The name of the color of this <code>Eater</code>
	 */
	public Eater(Location loc, EaterControl control, String color){
		myLocation = loc;
		myEC = control;
		myColorName = color;
	}
	
	/**
	 * Gives caller a 5 * 5 2D array containing the <code>Object</code>s within 2 squares to
	 * the left, right, above, and below this <code>Eater</code>.
	 * @return A 2D array containing the information that this <code>Eater</code> can see.
	 */
	final public Object[][] getVisibleSquares(){
		if(lastTurnVisibleSquaresCalculated == myEC.getWorldCount()
				&& lastLocationVisibleSquaresCalculated.equals(myLocation)){
			return(lastVisibleSquares);
		}
		lastVisibleSquares = null;
		lastVisibleSquares = new Object[5][5];
		for(int x = 0; x < 5; ++x){
			for(int y = 0; y < 5; ++y){
				lastVisibleSquares[x][y] = myEC.getLocationContents(myLocation.getX() - 2 + x, myLocation.getY() - 2 + y);
			}
		}
		lastLocationVisibleSquaresCalculated = myLocation;
		lastTurnVisibleSquaresCalculated = myEC.getWorldCount();
		return(lastVisibleSquares);
	}
	
	/**
	 * {@inheritDoc}
	 * This method returns the next decision the <code>Eater</code> will make by
	 * calling <code>getDecision</code>. The <code>Object</code> returned
	 * is an instance of <code>EaterOutputInfo</code> representing the decision.
	 * This method also increments the move count for this <code>Eater</code> if
	 * the decision is one to move or jump.
	 * @return The decision made, wrapped in an <code>EaterOutputInfo</code>.
	 * @see Eater#getDecision(EaterInputInfo)
	 */
	final public Object makeDecision(){
		
		EaterOutputInfo output;
		EaterInputInfo sensors = new EaterInputInfo(this);
		fillSensors(sensors);
		fireMakingDecisionNotification(sensors);
		
		if (this instanceof HumanEater)
			output = ((HumanEater)this).getDecision(sensors); 
		else
			output = processOutputLink();
		
		if (output.move != null || output.jump != null) {
			++myMoveCount;
			fireMoveCountChangedNotification();
		}
		
		return(output);
	}
	
	/**
	 * Gets the Eater agent's output link information, parses it into an
	 * EaterOutputInfo struct, and clears all output link changes.
	 * 
	 * @return EaterOutputInfo Eater's output information
	 */
	public EaterOutputInfo processOutputLink()
	{
			EaterOutputInfo info = new EaterOutputInfo();

			Identifier targetId = null;
			boolean goodOutput = false;
			boolean commandMissing = false;

			Identifier oLink = agent.GetOutputLink();

			int numCommands = agent.GetNumberCommands();

			for (int commandNum = 0; commandNum < numCommands; ++commandNum) {
				Identifier commandId = agent.GetCommand(commandNum);
				String commandName = commandId.GetAttribute();

				if (commandName.equals(MOVE)) {
					//Handle move
					String paramValue = commandId
							.GetParameterValue("direction");
					if (paramValue != null) {
						info.move = new EaterOutputInfo.MoveOutput(
								paramValue);
						commandId.AddStatusComplete();
					}
				} else if (commandName.equals(JUMP)) {
					//Handle jump
					String paramValue = commandId
							.GetParameterValue("direction");
					if (paramValue != null) {
						info.jump = new EaterOutputInfo.JumpOutput(
								paramValue);
						commandId.AddStatusComplete();
					}
				}
			}

			agent.ClearOutputLinkChanges();
			
			return info;
	}

	/**
	 * Fills the sensors that are contained in a <code>EaterOutputInfo</code>
	 * that the <code>Eater</code> controls.
	 * 
	 * @param sensors
	 *            The <code>EaterInputInfo</code> that is being filled with the
	 *            resulting calculations.
	 * @return The <code>EaterInputInfo</code> that has been filled with the
	 *         information available to this <code>Eater</code>.
	 */
	public EaterInputInfo fillSensors(EaterInputInfo sensors) {
		sensors = new EaterInputInfo(this);
		
		return (sensors);
	}

	/**
	 * Tells the caller the x and y coordinate distances of the specified
	 * location from this Eater.
	 * 
	 * @param loc
	 *            The location whose distance we wish to find.
	 * @return A Location whose getX() method will return the x distance from
	 *         this Eater, getY() method will return the y distance from the
	 *         Eater.
	 */
	final public Location distanceFrom(Location loc){
		int x = loc.getX() - myLocation.getX();
		int y = loc.getY() - myLocation.getY();
		return(new Location(x, y));
	}
	
	/**
	 * Returns the number of moves that this eater has taken to the caller.
	 * @return Integer number of moves that this Eater has taken.
	 */
	public int getNumberMoves(){
		return(myMoveCount);
	}
	
	/**
	 * Returns the Eater's color as a string
	 * @return color name as string
	 */
	public String getColorName(){
		return (myColorName);
	}
	
	/**
	 * Returns the location, an x,y coordinate class, of this eater.
	 * @return Location of the Eater.
	 */
	public Location getLocation(){
		return myLocation;
	}
	
	/**
	 * Sets the location of this eater to be that specified.
	 * @param loc The new Location of the eater.
	 */
	public void setLocation(Location loc){
		if(!myLocation.equals(loc)){
			myLocation = loc;
			fireLocationChangedNotification();
		}
	}
	
	/**
	 * Returns the score of this instance of Eater.
	 */
	public int getScore() { return myScore; }

	/**
	 * Sets the score of this eater to be newScore.
	 * @param newScore The new integer valued score to be the score of this eater.
	 */
	public void setScore(int newScore){
		if(myScore != newScore){
			myScore = newScore;
			fireScoreChangedNotification();
		}
	}
	
	public void reset(){
		setScore(0);
		myRecentDecision = NoDecision;
		myMoveCount = 0;
		fireMoveCountChangedNotification();
	}
	
	/**
	 * Returns to the caller the integer value corresponding to the last decision made
	 * by the eater.
	 * @return Integer value of the last decision made.<p><p>Possible <b>return</b> values:
	 * <p><t>Eater.MoveNorth, Eater.MoveEast, Eater.MoveSouth, Eater.MoveWest,
	 * Eater.JumpNorth, Eater.JumpEast, Eater.JumpSouth, Eater.JumpWest, and Eater.NoDecision.
	 */
	public int getRecentDecision(){
		return (myRecentDecision);
	}
	
	/**
	 * Method invoked when Eater has landed on a piece of normal food. Uses
	 * EaterControl.NormalFoodWorth to determine what score to add.
	 */
	public void ateNormalFood()
	{
		if (myEC.commandLine.logVerbosity >= 3)
			myEC.logger.log(getName() + " ate normal food at (" + getLocation().getX()
					+ "," + getLocation().getY() + ")\n");
		
		myScore += EaterControl.NormalFoodWorth;
		myEC.foodEaten(EaterControl.NormalFood);
		if(EaterControl.NormalFoodWorth != 0){
			fireScoreChangedNotification();
		}
	}
	/**
	 * Method invoked when Eater has landed on a piece of bonus food. Uses
	 * EaterControl.BonusFoodWorth to determine what score to add.
	 */
	public void ateBonusFood()
	{
		if (myEC.commandLine.logVerbosity >= 2)
			myEC.logger.log(getName() + " ate bonus food at (" + getLocation().getX()
					+ "," + getLocation().getY() + ")\n");
		
		myScore += EaterControl.BonusFoodWorth;
		myEC.foodEaten(EaterControl.BonusFood);
		if(EaterControl.BonusFoodWorth != 0){
			fireScoreChangedNotification();
		}
	}
	/**
	 * Method invoked when <code>Eater</code> has jumped. Uses
	 * <code>EaterControl.JumpCost</code> to determine what score to add.
	 */
	public void jumped()
	{
		myScore -= EaterControl.JumpCost;
		if(EaterControl.JumpCost != 0){
			fireScoreChangedNotification();
		}
	}
	
	public int getWorldCount(){
		return(myEC.getWorldCount());
	}
	
	/**
	 * Method invoked by EaterControl when this instance of Eater is destroyed.
	 * Generally overridden by subclasses.
	 */
	public void destroyEater(){
		
	}
	
	public String getName(){
		return(getColorName());
	}
	
	/**
	 * Adds the given EaterListener to the internal list of EaterListeners of this
	 * instance of Eater.
	 * @param el The EaterListener to be added.
	 */
	public void addEaterListener(EaterListener el){
		if(eListeners == null) eListeners = new ArrayList();
		eListeners.add(el);
	}
	
	/**
	 * Removes the specified EaterListener to the internal list of EaterListeners of
	 * this instance of Eater. Does nothing if rem is not a listener when this method
	 * is invoked..
	 * @param rem The EaterListener to be removed.
	 */
	public void removeEaterListener(EaterListener rem){
		if(eListeners == null) return;
		eListeners.remove(rem);
	}
	
	public boolean equals(Object o){
		return (o == this ||
				(o instanceof Eater && 
						this.getColorName().equals(((Eater)o).getColorName())));
	}
	
	/*----- LISTENER NOTIFICATIONS -----*/
	
	private void fireScoreChangedNotification(){
		if(eListeners == null) return;
		Iterator iter = eListeners.iterator();
		while(iter.hasNext()){
			((EaterListener)iter.next()).scoreChanged(this);
		}
	}
	
	private void fireLocationChangedNotification(){
		if(eListeners == null) return;
		Iterator iter = eListeners.iterator();
		while(iter.hasNext()){
			((EaterListener)iter.next()).eaterLocationChanged(this);
		}
	}
	
	private void fireMoveCountChangedNotification(){
		if(eListeners == null) return;
		Iterator iter = eListeners.iterator();
		while(iter.hasNext()){
			((EaterListener)iter.next()).moveCountChanged(this);
		}	
	}
	
	private void fireMakingDecisionNotification(EaterInputInfo sensors){
		if(eListeners == null) return;
		Iterator iter = eListeners.iterator();
		while(iter.hasNext()){
			((EaterListener)iter.next()).makingDecision(this, sensors);
		}
	}
	
	/**
	 * Load a Soar agent and attach it to the Eater.
	 * @param toAttach File object representing .soar file to attach
	 */
	public void attachSoarCode(File toAttach) {	
		if (kernel == null) {
			JOptionPane.showMessageDialog(null, "Fatal Error: Kernel was null in Eater::attachSoarCode(File to Attach).","Eaters",
	                JOptionPane.ERROR_MESSAGE);
			System.exit(-1);
			return;
		}
		String agentName = toAttach.getName();
		//use the production name, stopping before '.'
		agentName = agentName.substring(0, agentName.indexOf('.'));
		agentName = this.getColorName() + "-" + agentName + String.valueOf(agentNumber++);
		//create the new agent.
		agent = kernel.CreateAgent(agentName);
		if (agent == null) {
			JOptionPane.showMessageDialog(null, "Fatal Error: " + kernel.GetLastErrorDescription(),"Eaters",
	                JOptionPane.ERROR_MESSAGE);
	          System.exit(-1);
		}
		
		String[] pathAndFilename = toAttach.getPath().split(agentName);
		String pathOnly = pathAndFilename[0];
		
		iLink = new EaterInputLink(agent);
		
		productionPath = toAttach.getPath();

		if (!agent.LoadProductions(productionPath)) {
			JOptionPane.showMessageDialog(null, "Fatal Error: Productions for agent " + agentName +
					" did not load correctly.\nReason: " + agent.GetLastErrorDescription() + "\nand with a commandline result of " +
					kernel.GetLastCommandLineResult(),"Eaters",
	                JOptionPane.ERROR_MESSAGE);
			System.exit(-1);
		}
	}
	
	public String getProductionPath() {
		if (productionPath.length() == 0) return null;
		return productionPath;
	}

	/**
	 * {@inheritDoc}
	 * @return In our case, the <code>Eater</code>'s color name, location, and score.
	 */
	public String toString(){
		return ("Eater: " + myColorName + " at " + myLocation + " score: " + myScore);
	}

	/**
	 * If this is not a human eater, we must update the Soar agent's input link
	 */
	public void updateSensors() {
		if (!(this instanceof HumanEater))
			iLink.update(new EaterInputInfo(this));		
	}
	
	/**
	 * Set the Eater's kernel (necessary prior to calling attachSoarCode)
	 * 
	 * @param nkernel kernel to attach
	 */
	public void setKernel(Kernel nkernel) {
		if (nkernel != null)
			kernel = nkernel;
		else
		{
			JOptionPane.showMessageDialog(null, "Fatal Error: attempted to set an Eater with a null kernel.","Eaters",
	                JOptionPane.ERROR_MESSAGE);
	          System.exit(-1);
		}
	}
}