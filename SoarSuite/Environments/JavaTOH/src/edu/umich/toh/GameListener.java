//-----------------------------------------------------------------------------------
// edu.umich.toh.GameListener
// Author: Trevor McCulloch
//   Date: 16 November 2004
//-----------------------------------------------------------------------------------

package edu.umich.toh;


/**
 * The <code>GameListener</code> interface provides delegation for two common events
 * in the <code>Game</code> class: disk movement (from one <code>Tower</code> to
 * another), and game completion.
 * 
 * @author Trevor McCulloch, University of Michigan
 * @version 0.1
 * 
 * @see Game
 */
public interface GameListener extends sml.Kernel.SystemEventInterface {
    /**
     * Whenever a disk is moved from one tower to another, this method is called to
     * inform this listener where the disk was moved from and where it was moved to
     * in the specified <code>Game</code>.
     * 
     * @param g            the <code>Game</code> a disk was moved in.
     * @param source       the <code>Tower</code> the disk was removed from.
     * @param destination  the <code>Tower</code> the disk was moved to.
     *
     * @see Tower
     */
    public void diskMoved(Game g, Tower source, Tower destination);
    
    /**
     * When the specified <code>Game</code> reaches the goal state (all of the disks
     * are stacked smallest to largest on the right-most tower), this method is
     * called to inform this listener. This method will only be called once in the
     * life span of the specified <code>Game</code>.
     * 
     * @param g  the <code>Game</code> that reached to goal state.
     */
    public void atGoalState(Game g);
}
