/* File: SWindowManager.java
 * Jul 16, 2004
 */

package edu.umich.JavaBaseEnvironment;

import java.util.*;

import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.*;

/**
 * Class that contains the windows for a Soar simulation, including a visible map,
 * agent windows that describe what it is the agent sees, and possibly control
 * windows for any human-controlled agents.
 * @author John Duchi
 */
public class SWindowManager implements SimulationControlListener, WorldCountNotificationListener{

	/** The <code>String</code> name of the directory from which this simulation was
	 * launched, especially useful in determining image directories. */
	protected String startingDirectory = System.getProperty("user.dir");
	/** The system-dependant file separator character. */
	private String sep = System.getProperty("file.separator");
	/** The extra part of the directory path that leads to the images for the simulation
	 * (after the path in startingDirectory). */
	protected String imagePath = sep + "images" + sep;
	protected SimulationControl mySC;
	protected boolean agentWindowsOn = true;
	private boolean mapWindowOn = true;
	private boolean controlPanelOn = true;
	protected ArrayList agentWindows = null;
	private ArrayList agentControlWindows = null;
	protected AgentWindow myVisualMap = null;
	private ControlPanel myCP = null;
	protected Display myDisplay;
	private boolean disablePopups;

	public static Color white;
	public static Color blue;
	public static Color red;
	public static Color widget_background;
	public static Color yellow;
	public static Color orange;
	public static Color black;
	public static Color green;
	public static Color purple;
    	
	/**
	 * Initializes an SWindowManager, a class that manages displays for a simulation.
	 * Maintains a control panel, a map window, and agent windows.
	 * @param sc The SimulationControl that is controlling the simulation.
	 * @param agentWindowsOn If true, creates agent windows when new agents are created.
	 * @param mapWindowOn If true, will show the map window of the simulation.
	 * @param controlPanelOn
	 */
	public SWindowManager(SimulationControl sc, boolean agentWindowsOn, boolean mapWindowOn, 
			boolean controlPanelOn,boolean disablePopups,String taskIn, 
			String taskNounIn, String extIn, String extDesIn){
		mySC = sc;
		this.disablePopups = disablePopups;
		this.agentWindowsOn = agentWindowsOn;
		this.mapWindowOn = mapWindowOn;
		this.controlPanelOn = controlPanelOn;
		myDisplay = new Display();
		mySC.addSimulationControlListener(this);
		mySC.addWorldCountNotificationListener(this);
		initializeVisuals(taskIn, taskNounIn, extIn, extDesIn);
	}
    	
	/**
	 * Initializes an SWindowManager with all windows set to be on (same as calling
	 * SWindowManager(ec, true, true, true)).
	 * @param sc The SimulationControl that is controlling the simulation.
	 */
	public SWindowManager(SimulationControl sc, String taskIn, String taskNounIn, String extIn, String extDesIn){
		mySC = sc;
		mySC.addSimulationControlListener(this);
		myDisplay = new Display();
		initializeVisuals(taskIn, taskNounIn, extIn, extDesIn);
	}
	
	/**
	 * Initializes the static colors for any graphics windows that need them.
	 * Note that this should not be called if there has already been a display
	 * allocated.
	 */
	public static void initColors(Display d){
	    if(white != null) return;
	    white = d.getSystemColor(SWT.COLOR_WHITE);
		widget_background = d.getSystemColor(SWT.COLOR_WIDGET_BACKGROUND);
		blue = d.getSystemColor(SWT.COLOR_BLUE);
		red = d.getSystemColor(SWT.COLOR_RED);
		yellow = d.getSystemColor(SWT.COLOR_YELLOW);
		green = d.getSystemColor(SWT.COLOR_GREEN);
		purple = d.getSystemColor(SWT.COLOR_DARK_MAGENTA);
		orange = new Color(d, 255, 127, 0);
		black = d.getSystemColor(SWT.COLOR_BLACK);
	}
	
	/**
	 * Called when simulation is quit, this disposes the colors allocated directly 
	 * from the operating system.
	 */
	private void disposeColors(){
		if(white == null) return;
		orange.dispose();
		//We do not need to dispose any other colors, since they are all allocated by the display.
	}
	
	/**
	 * Initializes all the GUI parts of a Soar simulation (i.e. TankSoar).
	 * Initializes the control panel, if the map or agent windows are on, initializes
	 * those as well. Opens them, and then has the <code>Display</code> sleep until
	 * it is dispatched.
	 */
	private void initializeVisuals(String task, String taskNoun, String ext, String extDes){
		initColors(myDisplay);
		if (controlPanelOn)
		{
			myCP = new ControlPanel(mySC, task, taskNoun, myDisplay, ext, extDes);
			myCP.open();
			myCP.setManager(this);
		}
		if(mapWindowOn){
			openMap();
		}
		if(agentWindowsOn){
			SoarAgent[] agents = mySC.getAllAgents();
			for(int i = 0; i < agents.length; ++i){
				openAgentView(agents[i]);
			}
		}
		while(!allShellsDisposed()){
		    if(!myDisplay.readAndDispatch()){
		        myDisplay.sleep();
		    }
		}
		disposeColors();
		myDisplay.dispose();
	}
	
	/**
	 * Opens the visible map of the simulation.
	 */
	public void openMap(){/*stokesd  Child classes do their own thing now*/}
	
	/**
	 * Determines whether all the shells managed by the SWindowManager have been closed and/or
	 * disposed. Well, that's ostensibly what it does. In reality, only cares about the
	 * control panel. If the <code>ControlPanel</code> is open, the shells are not
	 * disposed. Otherwise, they might or might not be.
	 * @return <b>true</b> if all managed shells are closed, <b>false</b> otherwise.
	 */
	public boolean allShellsDisposed(){
		if(myVisualMap != null && myVisualMap.isOpen()){ return (false); }
		if(myCP != null && myCP.isOpen()){ return (false); }
		return true;
	}
	
	/**
	 * Closes the visible map, if it is open.
	 */
	public void closeMap(){
		if(myVisualMap == null) return;
		myVisualMap.close();
	}

	/**
	 * Opens the given <code>SoarAgent</code>'s viewing window.
	 * Iterates over all the open <code>AgentWindow</code>s, opening the one
	 * whose <code>getAgent()</code> method returns the <code>SoarAgent</code>
	 * at <code>agent</code>. If none return <code>agent</code>, will create
	 * a new <code>AgentView</code> dependant on the type of simulation being run,
	 * adding it to the <code>agentWindows</code> collection.
	 * @param agent The <code>SoarAgent</code> whose view we wish to open.
	 */
	public void openAgentView(SoarAgent agent){/*stokesd  Child classes do their own thing now*/}
	
	public void closeAgentView(SoarAgent agent){
		if(agentWindows == null) return;
		Iterator iter = agentWindows.iterator();
		AgentWindow aw = null;
		SoarAgent sa = null;
		while(iter.hasNext()){
			aw = (AgentWindow)iter.next();
			sa = aw.getAgent();
			if(sa.getName().equals(agent.getName())){
				break;
			} else {
				sa = null;
			}
		}
		if(sa != null){
			aw.close();
		}
	}
	
	/**
	 * Gives the caller a color that corresponds to the named color.
	 * @throws SWTException if the colors managed by the SWindowManager have not been
	 * initialized, because no class has made use of an instance of SWindowManager.
	 * @param colorName The String name of the color sought.
	 * @return The color (as in SWT) named by colorName, or the color widget_background
	 * if no color is found to match the name passed in.
	 */
	public static Color getNamedColor(String colorName){
		if(widget_background == null){
		    throw new SWTException("Colors not initialized");
		}
	    if(colorName == null) return(widget_background);
		if(colorName.equals(AgentColors.red)) return(red);
		if(colorName.equals(AgentColors.black)) return(black);
		if(colorName.equals(AgentColors.green)) return(green);
		if(colorName.equals(AgentColors.blue)) return(blue);
		if(colorName.equals(AgentColors.yellow)) return(yellow);
		if(colorName.equals(AgentColors.orange)) return(orange);
		if(colorName.equals(AgentColors.purple)) return(purple);
		return(widget_background);
	}
	
	/**
	 * Creates a human-controlled agent in the simulation. Notifies the simulation's
	 * control that this should occur, then opens the view of information visible
	 * to normal agents in the simulation.
	 * @param colorName The String name of the color of human agent to create.
	 */
	public void createHumanAgent(String colorName){/*stokesd  Child classes do their own thing now*/}
	
	public void simEnded(final String message){
		if (!disablePopups)
		{
			if(message != null && myDisplay != null){
				myDisplay.asyncExec(new Runnable(){
					public void run(){
						MessageBox mb = new MessageBox(new Shell(myDisplay), SWT.OK | SWT.ICON_INFORMATION);
						mb.setMessage(message);
						mb.setText("Simulation Stopped");
						mb.open();
					}
				});
			}
		}
	}
	
	public void simQuit(){
		
	}
	
	public void simStarted(){
		
	}
	
	public void locationChanged(Location loc){
		
	}
	
	public void agentCreated(SoarAgent created){
		/* Ignored because if we open an agent view here, we get a ConcurrentModificationException */
	}
	
	public void agentDestroyed(SoarAgent destroyed){
		if(agentWindows == null) return;
		Iterator iter = agentWindows.iterator();
		AgentWindow aw = null;
		SoarAgent sa = null;
		while(iter.hasNext()){
			aw = (AgentWindow)iter.next();
			sa = aw.getAgent();
			if(sa.getName().equalsIgnoreCase(destroyed.getName())){
				break;
			} else {
				sa = null;
			}
		}
		if(sa != null){
			iter.remove();
			aw.close();
		}
	}
	
	public void newMap(String message){
		if (!disablePopups)
		{
			if(message == null) return;
			MessageBox mb = new MessageBox(new Shell(myDisplay), SWT.OK | SWT.ICON_INFORMATION);
			mb.setMessage(message);
			mb.setText("Soar information");
			mb.open();
		}
	}
	
	public void finishedWorldCountUpdating(){
		
		if(myCP.isOpen()){
			mySC.waitATouch(myCP.getSpeed()*25);
		}
	}
	
	public void worldCountChanged(int newWorldCount){

	}
}
