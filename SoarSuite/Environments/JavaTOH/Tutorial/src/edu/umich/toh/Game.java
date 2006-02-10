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
    }
    
    /** Run one decision cycle, which is usually enough to move one tile */
    public void step()
    {
		 
    	if (isAtGoalState())
    		return ;
    }
    
    /** Stop a run (might have been started here in the environment or in the debugger) */
    public void stop()
    {    	
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
}
