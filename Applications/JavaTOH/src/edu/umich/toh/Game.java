//------------------------------------------------------------------------------
// edu.umich.toh.Game
// Author: Trevor McCulloch
//   Date: 16 November 2004
//------------------------------------------------------------------------------

package edu.umich.toh;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;

import sml.Agent;
import sml.Identifier;
import sml.Kernel;
import sml.smlAgentEventId;
import sml.smlRunEventId;
import sml.smlRunFlags;
import sml.smlSystemEventId;


/**
 * <code>Game</code> represents the entire environment for the Towers of Hanoi game:
 * the towers and disks used to play the game. Users can specify the number of towers
 * and disks used to play the game, ask Soar to calculate the next move, and examine
 * the current state of the game. Use the <code>run</code> method to run a single
 * decision cycle.
 * 
 * @author Trevor McCulloch, University of Michigan
 * @version 1.1
 */
public class Game implements Runnable, Agent.RunEventInterface, Kernel.UpdateEventInterface {
    /**
     * Constructor that creates a <code>Game</code> with the default configuration
     * of 3 towers and 11 disks.
     *
     * @see #Game(int, int)
     */
    public Game()	{ this(3, 11); }
        
    /**
     * Constructor that creates a <code>Game</code> with the specified number of
     * towers and disks. All the disks are initially on the first tower, stacked
     * from the smallest size on top to the largest size on the bottom.
     * 
     * @param towerCount  the number of towers used in the game. Should be >= 3.
     * @param diskCount   the number of disks used in the game. The disks created
     *                    have sizes from 1 to diskCount.
     */
    public Game(int towerCount, int diskCount) {
        try {
            // create our Soar kernel
            kernel = Kernel.CreateKernelInNewThread("SoarKernelSML");
        } catch (Exception e) {
            System.out.println("Exception while creating kernel: " + e.getMessage());
            System.exit(1);
        }
        
        if (kernel.HadError())
        {
        	System.out.println("Error creating kernel: " + kernel.GetLastErrorDescription()) ;
            System.exit(1);
        }
        
        m_StopNow = false ;
        
        agent = kernel.CreateAgent(AGENT_NAME);
        boolean load = agent.LoadProductions("towers-of-hanoi-SML.soar");
        if (!load || agent.HadError()) {
            throw new IllegalStateException("Error loading productions: "
                    + agent.GetLastErrorDescription());
        }

        // Register for the event we'll use to update the world
        registerForUpdateWorldEvent() ;

        this.diskCount = diskCount;
        
        for (int i = 0; i < towerCount; ++i) {
            char data[] = new char[1];
            data[0] = (char)(i + 'A');
            String name = new String(data);
            
            towers.add(new Tower(this, name));
        }
        
        Tower t = (Tower)towers.get(0);
        for (int i = diskCount; i > 0; --i)
            t.addDisk(new Disk(t, i, t.getTopDisk()));
        
        agent.Commit();
    }
    
    /*********************************************************************************************
     * 
     * Update the state of the environment.
     * 
     * This involves:
     * a) Collecting any output the agent has sent since the last update
     * b) Changing the world state
     * c) Sending new input to the agent
     * 
    ********************************************************************************************/
    public void updateWorld()
    {
        // See if any commands were generated on the output link
    	// (In general we might want to update the world when the agent
    	// takes no action in which case some code would be outside this if statement
    	// but for this environment that's not necessary).
        if (agent.Commands())
        {
	        // perform the command on the output link        
	        Identifier command = agent.GetCommand(0);
	        if (!command.GetCommandName().equals(MOVE_DISK)) {
	            throw new IllegalStateException("Unknown Command: "
	                    + command.GetCommandName());
	        }
	        if (command.GetParameterValue(SOURCE_PEG) == null ||
	                command.GetParameterValue(DESTINATION_PEG) == null) {
	            throw new IllegalStateException("Parameter(s) missing for Command "
	                    + MOVE_DISK);
	        }
	        int srcPeg = command.GetParameterValue("source-peg").charAt(0) - 'A';
	        int dstPeg = command.GetParameterValue("destination-peg").charAt(0) - 'A';
	        
	        // Change the state of the world and generate new input
	        moveDisk(srcPeg, dstPeg);
	        
	        // Tell the agent that this command has executed in the environment.
	        command.AddStatusComplete();

	        // Send the new input link changes to the agent
	        agent.Commit();
        
	    	// "agent.GetCommand(n)" is based on watching for changes to the output link
	    	// so before we do a run we clear the last set of changes.
	    	// (You can always read the output link directly and not use the
	    	//  commands model to determine what has changed between runs).
	    	agent.ClearOutputLinkChanges() ;

	        if (isAtGoalState())
	            fireAtGoalState();
        }    	
    }
    
    /** This method is called when the "after_all_output_phases" event fires, at which point we update the world */
	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags)
	{
		try
		{
			// We have a problem at the moment with calling Stop() from arbitrary threads
			// so for now we'll make sure to call it within an event callback.
			if (m_StopNow)
			{
		    	m_StopNow = false ;	    	
		    	kernel.StopAllAgents() ;
			}
	
			/* Example of flags we may be passed.  In other situations
			 * we might choose not to update the world in some of these situations
			 * (e.g. for run --self unless also had --update).
			if ((runFlags & smlRunFlags.sml_RUN_SELF.swigValue()) != 0)
				System.out.println("Called with run --self") ;
			if ((runFlags & smlRunFlags.sml_RUN_ALL.swigValue()) != 0)
				System.out.println("Called with run") ;
			if ((runFlags & smlRunFlags.sml_UPDATE_WORLD.swigValue()) != 0)
				System.out.println("Called with run --update") ;
			if ((runFlags & smlRunFlags.sml_DONT_UPDATE_WORLD.swigValue()) != 0)
				System.out.println("Called with run --noupdate") ;
			*/
			
			updateWorld() ;
		}
		catch (Throwable t)
		{
			System.out.println("Caught a throwable event" + t.toString()) ;			
		}
	}

	public void runEventHandler(int eventID, Object data, Agent agent, int phase)
	{
		try
		{
			// We have a problem at the moment with calling Stop() from arbitrary threads
			// so for now we'll make sure to call it within an event callback.
			if (m_StopNow)
			{
		    	m_StopNow = false ;	    	
		    	kernel.StopAllAgents() ;
			}
	
			/* Example of flags we may be passed.  In other situations
			 * we might choose not to update the world in some of these situations
			 * (e.g. for run --self unless also had --update).
			if ((runFlags & smlRunFlags.sml_RUN_SELF.swigValue()) != 0)
				System.out.println("Called with run --self") ;
			if ((runFlags & smlRunFlags.sml_RUN_ALL.swigValue()) != 0)
				System.out.println("Called with run") ;
			if ((runFlags & smlRunFlags.sml_UPDATE_WORLD.swigValue()) != 0)
				System.out.println("Called with run --update") ;
			if ((runFlags & smlRunFlags.sml_DONT_UPDATE_WORLD.swigValue()) != 0)
				System.out.println("Called with run --noupdate") ;
			*/
			
			updateWorld() ;
		}
		catch (Throwable t)
		{
			System.out.println("Caught a throwable event" + t.toString()) ;			
		}
	}

    /**
     * Runs Soar until interrupted.  Soar repeatedly chooses the next <code>Disk</code>
     * to be moved. Listeners will be notified as to which <code>Tower</code> the
     * <code>Disk</code> will be moved from and to.
     * 
     * @see Tower
     * @see Disk
     */
    public void run()
    {
    	if (isAtGoalState())
    		return ;
    	
    	m_StopNow = false ;

    	// Start a run
    	// (since this is single agent could use agent.RunSelfForever() instead, but this shows how to run multi-agent environments)
    	kernel.RunAllAgentsForever() ;
    }
    
    /** Run one decision cycle, which is usually enough to move one tile */
    public void step()
    {
    	if (isAtGoalState())
    		return ;

    	// Run one decision
    	kernel.RunAllAgents(1) ;
    }
    
    /** Stop a run (might have been started here in the environment or in the debugger) */
    public void stop()
    {    	
    	// We'd like to call StopSoar() directly from here but we're in a different
    	// thread and right now this waits patiently for the runForever call to finish
    	// before it executes...not really the right behavior.  So instead we use a flag and
    	// issue StopSoar() in a callback.
    	m_StopNow = true ;
    }
    
    /**
     * Resets this Game to it's initial state. This means removing all of the disks
     * from all of the towers and placing them all on in order on the first tower,
     * with the smallest disk on top and the largest disk on the bottom.
     */
    public void reset() {
        List disks = new ArrayList(diskCount);
        
        // collect all of the disks
        ListIterator i = towers.listIterator();
        while (i.hasNext()) {
            Tower t = (Tower)i.next();
            while (!t.isEmpty()) {
                disks.add(t.getTopDisk());
                t.removeTopDisk();
            }
        }
        
        // sort them
        Collections.sort(disks);
        
        // place them back on the first tower in reverse order
        Tower t = (Tower)towers.get(0);
        i = disks.listIterator(disks.size());
        while (i.hasPrevious())
            t.addDisk((Disk)i.previous());
        disks.clear();
        
        agent.Commit();
        
        // should allow us to run again.
        agent.InitSoar(); // causes near-infinite recusion in Kernel!
    }
    
    /**
     * Gets the number of <code>Disk</code> objects on all the <code>Tower</code>s
     * in this <code>Game</code>.
     * 
     * @return  the total number of <code>Disk</code>s.
     */
    public int getDiskCount()       { return diskCount;  }
    /**
     * Returns an unmodifiable <code>List</code> of this <code>Game</code>'s
     * <code>Tower</code>s. Towers are arranged from left to right, lowest index to
     * highest index. At creation of this <code>Game</code>, all <code>Disk</code>s
     * are on the <code>Tower</code> at index <code>0</code>, when
     * <code>isAtGoalState</code> returns <code>true</code>, all <code>Disk</code>s
     * are on the <code>Tower</code> at index <code>getDiskCount() - 1</code>.
     * 
     * @return  a list of all of this <code>Game</code>'s <code>Tower</code>s.
     */
    public List getTowers()     { return Collections.unmodifiableList(towers); }
    /**
     * Get this Game's Soar agent.
     * 
     * @return  this Game's Soar agent.
     */
    public Agent getAgent()     { return agent;       }
    
    /**
     * Queries to see if this <code>Game</code> is at the goal state, that is, all
     * disks are stacked on the last tower.
     * 
     * @return  <code>true</code> if all of the disks are on the last tower.
     */
    public boolean isAtGoalState() {
        return (((Tower)towers.get(towers.size() - 1)).getHeight() == diskCount);
    }
    
    /**
     * Adds the specified <code>GameListener</code> from this <code>Game</code>'s
     * listener list. Listeners will be notified when a disk is moved and when this
     * <code>Game</code> reaches the goal state.
     * 
     * @param l  <code>GameListener</code> to add to our listener list.
     */
    public void addGameListener(GameListener l)	{ listeners.add(l);    }
    /**
     * Removes the specified <code>GameListener</code> from this <code>Game</code>'s
     * listener list.
     * 
     * @param l  <code>GameListener</code> to remove from our listener list.
     */
    public void removeGameListener(GameListener l)	{ listeners.remove(l); }

    public void registerForStartStopEvents(GameListener listener)
    {
    	if (kernel != null)
    	{
			int startCallback = kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, listener, null) ;
			int stopCallback  = kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, listener, null) ;
    	}
    }
    
    /** We update the environment when this event fires.  This allows us to either run the environment directly or from a debugger and get correct behavior */
    public void registerForUpdateWorldEvent()
    {
    	int updateCallback = kernel.RegisterForUpdateEvent(sml.smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, this, null) ;
    	int eventCallback = agent.RegisterForRunEvent(smlRunEventId.smlEVENT_AFTER_DECISION_CYCLE, this, null) ;
    }

    public void detachSoar() {
    	kernel.DestroyAgent(agent);
    	kernel.Shutdown();
        kernel.delete();
    	System.out.println("All done") ;
    }
    
    public static final String MOVE_DISK       = "move-disk";
    public static final String SOURCE_PEG      = "source-peg";
    public static final String DESTINATION_PEG = "destination-peg";
    
    /**
     * Removes the top <code>Disk</code> from <code>sourceTower</code> and places it
     * on top of <code>destinationTower</code>.
     * 
     * @param sourceTower       the index of the source tower.
     * @param destinationTower  the index of the destination tower.
     */
    private void moveDisk(int sourceTower, int destinationTower) {
        Tower src = (Tower)towers.get(sourceTower);
        Tower dst = (Tower)towers.get(destinationTower);
        
        Disk d = src.getTopDisk();
        src.removeTopDisk();
        dst.addDisk(d);
        fireDiskMoved(src, dst);
    }
    
    /**
     * Notifies any registered listeners that a disk has been moved from
     * <code>source</code> tower to the <code>destination</code> tower.
     * 
     * @param source       the <code>Tower</code> a disk is being removed from.
     * @param destination  the <code>Tower</code> a disk is being added to.
     */
    protected void fireDiskMoved(Tower source, Tower destination) {
        Iterator i = listeners.iterator();
        while (i.hasNext())
            ((GameListener)i.next()).diskMoved(this, source, destination);
    }
    /**
     * Notifies any registered listeners that this <code>Game</code> has reached the
     * goal state.
     *
     */
    protected void fireAtGoalState() {
        Iterator i = listeners.iterator();
        while (i.hasNext())
            ((GameListener)i.next()).atGoalState(this);
    }
    
    public static final String AGENT_NAME = "TOH";
    
    private int diskCount;                     // total number of disks
    private List towers = new ArrayList();     // towers, from left to right
    private List listeners = new LinkedList(); // GameListeners
    
    private boolean m_StopNow ;
    
    // soar stuff
    private Kernel kernel;
    private Agent agent;
}
