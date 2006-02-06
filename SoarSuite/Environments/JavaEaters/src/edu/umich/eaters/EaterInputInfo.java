/* File: EaterInputInfo.java
 * Date: Sep 17, 2004
 */
package edu.umich.eaters;

/**
 * A simple struct-like class to contain the information available to an <code>Eater</code>
 * with which it can make a decision. Represents the input-link of an <code>Eater</code>
 * in the simulation.
 * @author John Duchi
 */
public class EaterInputInfo {
	
	  private Eater theEater;
	  
	/**
	 * Constructs a new instance of <code>EaterInputInfo</code> using the specified
	 * <code>Eater</code> as the <code>Eater</code> whose input-link this is
	 * representing. This will fill all the input-link attributes with
	 * the proper values when the <code>EaterInputInfo</code> is constructed.
	 * @param e The <code>Eater</code> whose input-link this <code>EaterInputInfo</code>
	 * represents.
	 */
	public EaterInputInfo(Eater e){
		theEater = e;
		visibleSquares = e.getVisibleSquares();
		eater = new EaterInput();
		eater.name = e.getName();
		eater.x = e.getLocation().getX();
		eater.y = e.getLocation().getY();
		eater.score = e.getScore();
	}
	
	/**
	 * A 2D array containing all the other <code>Object</code>s in the simulation that an
	 * <code>Eater</code> can see. Should be accessed as <code>visibleSquares[x-coordinate][y-coordinate]</code>,
	 * where [0][0] accesses the upper left corner, [4][4] the lower right.
	 * @see Eater#getVisibleSquares()
	 */
	public Object[][] visibleSquares = null;
	
	/**
	 * The ^eater attribute on the input-link.
	 */
	public static class EaterInput{
		
		/**
		 * The name of this <code>Eater</code> (actually, its color).
		 * <p><b>IO usage:</b> ^eater.name red/blue/green/black/yellow/orange/purple
		 */
		public String name = null;
		/**
		 * The x-coordinate of this <code>Eater</code>'s position.
		 * <p><b>IO usage:</b> ^eater.x 1-15
		 */
		public int x = 0;
		/**
		 * The y-coordinate of this <code>Eater</code>'s position.
		 * <p><b>IO usage:</b> ^eater.y 1-15
		 */
		public int y = 0;
		/**
		 * The score of the <code>Eater</code>.
		 * <p><b>IO usage:</b> ^eater.score 0-N
		 */
		public int score = 0;
		
	}
	/**
	 * The ^eater sensor on the input-link.
	 */
	public EaterInput eater;
	
}
