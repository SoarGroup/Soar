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
public class Game implements Runnable {
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

        agent = kernel.CreateAgent(AGENT_NAME);
        boolean load = agent.LoadProductions("towers-of-hanoi-SML.soar");
        if (!load || agent.HadError()) {
            throw new IllegalStateException("Error loading productions: "
                    + agent.GetLastErrorDescription());
        }
        
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
    
    /**
     * Notify any listeners that the system has now stopped running.
     * 
     * @see Tower
     * @see Disk
     */
    public void fireStopped()
    {
    	kernel.FireStopSystemEvent() ;
    }
    
    private void runAgent()
    {
    	// We're running the simulation until the user presses stop
    	// so we don't want the run command to send a system stop at the end
    	// (we'll do that manually once the user stops the simulation)
    	kernel.SuppressSystemStop(true) ;
    	
    	// "Commands" is based on a changes to the output link
    	// so before we do a run we clear the last set of changes.
    	// (You can always read the output link directly and not use the
    	//  "commands" model to determine what has changed between runs).
    	agent.ClearOutputLinkChanges() ;
    	
    	// Run the agent for a decision, possibly creating output.
    	agent.RunSelf(1) ;    	
    }
    
    /**
     * Runs one Soar decision cycle in which Soar chooses the next <code>Disk</code>
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
    	
        // See if any commands were generated on the output link
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
	        moveDisk(srcPeg, dstPeg);
	        
	        command.AddStatusComplete();
	        
	        agent.Commit();
        
	        if (isAtGoalState())
	            fireAtGoalState();
        }
        
        // run, so we get a Command on the OuputLink
        // We generally want to do this at the end of the loop so that if we break
        // into the debugger, we can step in the debugger (without advancing the simulation)
        // and any output created will be picked up when we next start the simulation.
        // This doesn't make much difference in TOH because each decision creates output,
        // but in an agent that reasoned more this would make a difference.
        runAgent() ;
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

    public void registerForStartStopEvents(GameListener listener, String methodName)
    {
    	if (kernel != null)
    	{
			int startCallback = kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, listener, methodName, null) ;
			int stopCallback  = kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, listener, methodName, null) ;
    	}
    }

    public void detachSoar() {
        kernel.DestroyAgent(agent);
        kernel.delete();
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
    
    // soar stuff
    private Kernel kernel;
    private Agent agent;
}
