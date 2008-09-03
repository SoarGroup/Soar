//------------------------------------------------------------------------------
// edu.umich.toh.Tower class.
// Author: Trevor McCulloch
//   Date: 9 November 2004
//------------------------------------------------------------------------------

package edu.umich.toh;

import java.util.ArrayList;
import java.util.List;

import sml.Agent;
import sml.StringElement;


/**
 * <code>Tower</code> represents a tower or peg in the Towers of Hanoi problem.
 * It is responsible for managing the WMEs related to the tower in Soar,
 * which is mostly the name of the tower. <code>Tower</code> also maintains a
 * list of <code>Disk</code>s that are on it, in the order they are stacked on
 * the tower.
 *
 * @author Trevor McCulloch, University of Michigan
 * @version 1.1
 */
public class Tower {
    /**
     * Constructor specifying which <code>Game</code> this <code>Tower</code>
     * is in, and the name of the <code>Tower</code>.
     *
     * @param game  the <code>Game</code> this <code>Tower</code> is in.
     * @param name  the name of this <code>Tower</code>.
     * @see Game
     */
    public Tower(Game game, String name) {
        this.name = name;
        this.game = game;
        
        Agent agent = game.getAgent();
        pegName = agent.CreateStringWME(agent.GetInputLink(), PEG, name);
    }
    
    /**
     * Adds the specified <code>Disk</code> to the top of this tower.
     */
    public void addDisk(Disk top) {
        top.setPosition(this, getTopDisk());
        disks.add(top);
    }
    
    /**
     * Gets the top <code>Disk</code>.
     *
     * @return  the top <code>Disk</code>
     */
    public Disk getTopDisk() {
        return (disks.isEmpty() ? null : (Disk)disks.get(disks.size() - 1));
    }
    
    /**
     * Removes the top <code>Disk</code>. Note that this method DOES NOT remove or
     * modify the top disk's WMEs in Soar.
     * Use <code>Disk.setPosition</code> for that.
     */
    public void removeTopDisk() { disks.remove(disks.size() - 1); }
    /**
     * Gets the name of this tower.
     * 
     * @return  the name of this tower.
     */
    public String getName()     { return name;	}
    /**
     * Gets the number of <code>Disk</code>s on this Tower.
     *
     * @return  the number of <code>Disk</code>s.
     */
    public int getHeight()      { return disks.size();  }
    /**
     * @return  <code>true</code> if there are no disks on this tower,
     *          <code>false</code> otherwise.
     */
    public boolean isEmpty()    { return disks.size() == 0; }
    /**
     * @return  the <code>Game</code> this Tower is in.
     */
    public Game getGame()       { return game; }
    /**
     * Returns a Soar-like representation of the peg, the only attribute of which is
     * the name of the peg.
     * 
     * @return a string representation of this Tower's Soar state.
     */
    public String toString() {
        return "(peg ^name " + name + ")";
    }
    
    protected static final String PEG  = "peg";
    
    // NOTE: don't worry about destroying WMEs, they'll go when the agent is
    // destroyed and the kernel is cleaned up
    
    private Game game;    // the game this belongs to
    private String name;  // name of the tower, should be a single letter
    private List disks = new ArrayList(); // disks on tower, higher index = higher up
    
    // Soar stuff
    private StringElement pegName;
}
