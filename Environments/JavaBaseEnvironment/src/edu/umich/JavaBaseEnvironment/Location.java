package edu.umich.JavaBaseEnvironment;
/* File: Location.java
 * Jul 12, 2004
 */

/**
 * Simple class that specifies an (x, y) location and will return locations
 * next adjacent to itself.
 * @author John Duchi
 */
public class Location {

	private int x;
	private int y;
	
	public static final int North = 0;
	public static final int East = 1;
	public static final int South = 2;
	public static final int West = 3;
	
	/**
	 * Creates a new instance of Location.
	 * @param x The x coordinate of the new Location.
	 * @param y The y coordinate of the new Location.
	 */
	public Location(int x, int y){
		this.x = x;
		this.y = y;
	}

	/**
	 * Creates a new instance of Location.
	 * @param loc The Location whose coordinates will be used as coordinates of this
	 * new Location.
	 */
	public Location(Location loc){
	    this.x = loc.getX();
	    this.y = loc.getY();
	}
	
	/**
	 * Returns the x coordinate of this Location instance.
	 */
	public int getX(){ return x; }
	/**
	 * Returns the y coordinate of this Location instance.
	 */
	public int getY(){ return y; }

	/**
	 * Returns a new Location instance in the direction specified.
	 * @param direction The direction in which to find the new Location.
	 * @return A new Location as specified by direction from this instance of Location.
	 */
	public Location getAdjacent(int direction){
		switch(direction){
		case(North): return new Location(x, y - 1);
		case East : return new Location(x + 1, y);
		case West: return new Location(x - 1, y);
		case South: return new Location(x, y + 1);
		
		default:
			throw new IndexOutOfBoundsException(direction + " is no good.");
		}
	}
	
	/**
	 * To be specific, returns <code>true</code> if
	 * <code>(this == o || (o instanceof Location && ((Location)o).x == this.x &&
	 * ((Location)o).y == this.y))</code>.
	 * {@inheritDoc}
	 */
	public boolean equals(Object o){
		return(o != null && (this == o ||
				(o instanceof Location && ((Location)o).x == this.x && ((Location)o).y == this.y)));
	}
	
	public String toString(){
		return ("Location: (" + x + ", " + y + ")");
	}
}
