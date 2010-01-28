//------------------------------------------------------------------------------
// edu.umich.toh.Disk class.
// Author: Trevor McCulloch
//   Date: 9 November 2004
//------------------------------------------------------------------------------

package edu.umich.toh;

/**
 * Disk represents a disk in the Towers of Hanoi problem. It is responsible for
 * managing the WMEs related to it in Soar, including which peg/tower the disk
 * is on, the size of the disk, and which other disk it is above.
 *
 * @author Trevor McCulloch, University of Michigan
 * @version 1.1
 */
public class Disk implements Comparable {
    /**
     * Constructor specifying which <code>Tower</code> the disk is on, the size
     * of the disk, and which disk is below it on the <code>Tower</code>.
     *
     * @param tower the <code>Tower</code> this disk is on.
     * @param size  the size of this disk.
     * @param below the <code>Disk</code> below this one on the
     *              <code>Tower</code>. Pass <code>null</code> if there are no
     *              disks below this one.
     * @see Tower
     */
    public Disk(Tower tower, int size, Disk below) {
        this.size  = size;
        this.tower = tower;
        this.below = below;
    }
    
    /**
     * Gets the size of this disk.
     *
     * @return  the size of this disk.
     */
    public int getSize()        { return size;  }
    /**
     * Gets the <code>Tower</code> this disk is on.
     *
     * @return  the <code>Tower</code> this disk is on.
     */
    public Tower getTower()     { return tower; }
    /**
     * Gets the <code>Disk</code> below this disk.
     * 
     * @return  the <code>Disk</code> below this disk on the tower.
     */
    public Disk getBelow()      { return below; }
    
    /**
     * Sets the position of this disk as being on the specified
     * <code>Tower</code> and above the specified <code>Disk</code>.
     *
     * @param tower  the <code>Tower</code> this disk is on.
     * @param below  the <code>Disk</code> below this one on the
     *               <code>Tower</code>.
     */
    public void setPosition(Tower tower, Disk below) {
        this.below = below;
        this.tower = tower;
    }
    
    /**
     * Determines if this disk is equal to another disk by comparing their size.
     * 
     * @param o - the object to compare this disk to.
     * @return <code>true</code> if <code>o</code> is a disk and is the same size as
     *         this disk, <code>false</code> otherwise.
     */
    public boolean equals(Object o) {
        return (o instanceof Disk ? size == ((Disk)o).size : false);
    }
    
    /**
     * Compares this Disk and another Disk by their size.
     * 
     * @param o - the disk to compare this disk to.
     * @return <code>this.size - o.size</code>.
     * @throws <code>ClassCastException</code>, if <code>!o instanceof Disk</code>.
     */
    public int compareTo(Object o) {
        return (size - ((Disk)o).size);
    }
    
    /**
     * Returns a Soar-like string representation of the disk, including the disk's
     * size, which peg it is on, and which disk it is above.
     * 
     * @return  a string representing this disk's Soar state.
     */
    public String toString() {
        StringBuffer sb = new StringBuffer("(disk ^size ");
        sb.append(size);
        sb.append(" ^on ");
        sb.append(tower.getName());
        sb.append(" ^above ");
        if (below != null)
            sb.append(below.getSize());
        else
            sb.append(NONE);
        sb.append(")");
        
        return sb.toString();
    }
    
    protected static final String DISK  = "disk";
    protected static final String NAME  = "name";
    protected static final String SIZE  = "size";
    protected static final String HOLDS = "holds";
    protected static final String ON    = "on";
    protected static final String ABOVE = "above";
    protected static final String NONE  = "none";
    
    private Tower tower;
    private int   size;
    private Disk  below;
}
