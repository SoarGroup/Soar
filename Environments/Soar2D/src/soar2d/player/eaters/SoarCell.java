package soar2d.player.eaters;

import java.util.HashMap;

import sml.Identifier;
import sml.StringElement;

/**
 * @author voigtjr
 *
 * represents a cell in the 5x5 agent view
 */
class SoarCell {
	/**
	 * current cell id
	 */
	Identifier me;
	/**
	 * the list of stuff in the cell
	 */
	HashMap<String, StringElement> content = new HashMap<String, StringElement>();
	
	/**
	 * box in current cell, null if none
	 */
	Identifier box;
	/**
	 * properties on the box if there is one
	 */
	HashMap<String, StringElement> boxProperties = new HashMap<String, StringElement>();

	/**
	 * id (likely shared) to the cell/wme to the north
	 */
	Identifier north;
	/**
	 * id (likely shared) to the cell/wme to the south
	 */
	Identifier south;
	/**
	 * id (likely shared) to the cell/wme to the east
	 */
	Identifier east;
	/**
	 * id (likely shared) to the cell/wme to the west
	 */
	Identifier west;
	
	/**
	 * used during initialization
	 */
	boolean iterated = false;
	
}

