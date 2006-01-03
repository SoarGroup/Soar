/* File: AgentColors.java
 * Jul 12, 2004
 */
package edu.umich.JavaBaseEnvironment;
import java.util.*;

/**
 * Class to manage the colors being used by an instance of EaterControl. Provides 7 different
 * colors to be used (at least the names of them).
 * It makes it more efficient for keeping track of the colors used for agents.
 * @author jduchi
 */
public class AgentColors {
	
	public static final String red = "red";
	public static final String green = "green";
	public static final String blue = "blue";
	public static final String orange = "orange";
	public static final String black = "black";
	public static final String purple = "purple";
	public static final String yellow = "yellow";
	
	public AgentColors(){
		initAgentColors();
	}
	
	/**
	 * Tells the caller whether the color named by the <code>String</code> specified may be used
	 * to create an agent.
	 * @param color The name of the color whose availability is to be checked.
	 * @return <code>true</code> if this <code>AgentColors</code> has not been notified of any
	 * agent using color, <code>false</code> otherwise. More specifically, returns
	 * <code>(color != null && colorsLeft.contains(color))</code>.
	 */
	public boolean colorAvailable(String color){
		return(color != null && colorsLeft.contains(color));
	}

	/**
	 * Tells the caller whether the color named by the String specified has already
	 * been used to create an agent.
	 * @param color The name of the color whose availability is to be checked.
	 * @return <code>false</code> if this <code>AgentColors</code> has not been notified of any
	 * agent using color, <code>true</code> otherwise. More specifically, returns
	 * <code>(color != null && colorsUsed.contains(color))</code>.
	 */
	public boolean colorUsed(String color){
		return (color != null && colorsUsed.contains(color));
	}
	
	/**
	 * Gives caller an array of strings of the colors that this AgentColors has
	 * been notified to use.
	 * @return Array of Strings of colors used by agents.
	 */
	public String[] colorsUsed(){
		String[] ss = (String[])colorsUsed.toArray(new String[0]);
		return (ss);
	}

	/**
	 * Gives caller an array of strings of the colors that this AgentColors has
	 * not been notified to use.
	 * @return Array of Strings of colors not in use by agents, as far as this
	 * AgentColors knows.
	 */
	public String[] colorsAvailable(){
		String[] ss = (String[])colorsLeft.toArray(new String[0]);
		return (ss);
	}
	
	private void initAgentColors(){
		colorsLeft.add(red);
		colorsLeft.add(green);
		colorsLeft.add(blue);
		colorsLeft.add(orange);
		colorsLeft.add(black);
		colorsLeft.add(purple);
		colorsLeft.add(yellow);
	}
	
	/**
	 * Method called to notify AgentColors that a color has been used
	 * in a simulation.
	 * @param color The name of the color used.
	 */
	public void useColor(String color){
		colorsLeft.remove(color);
		if(!colorsUsed.contains(color)){
		    colorsUsed.add(color);
		}
	}
	
	/**
	 * Method called to notify AgentColors that a color previously used
	 * in a simulation is no longer in use.
	 * @param color The name of the color to return to the set of unused
	 * colors.
	 */
	public void returnColor(String color){
		if(!colorsLeft.contains(color)){
		    colorsLeft.add(color);
		}
		colorsUsed.remove(color);
	}
	
	/**
	 * Randomly returns one of the unused colors in this AgentColors.
	 * @param r A random number generator.
	 * @return The name of a randomly selected color in this AgentColors.
	 */
	public String getRandomColorLeft(Random r){
		int size = colorsLeft.size();
		if(size == 0) return null;
		return ((String)colorsLeft.get(r.nextInt(colorsLeft.size())));
	}
	
	/** An <code>ArrayList</code> containing the names of all the colors that have been used
	 * thus far. */
	private ArrayList colorsUsed = new ArrayList();
	/** An <code>Arraylist</code> containing the names of the colors that have not been used
	 * thus far. */
	private ArrayList colorsLeft = new ArrayList();

}