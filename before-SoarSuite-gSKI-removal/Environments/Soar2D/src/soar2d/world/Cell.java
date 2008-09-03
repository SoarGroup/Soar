package soar2d.world;

import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

import soar2d.Names;
import soar2d.player.Player;

/**
 * @author voigtjr
 *
 * A cell in the grid based world.
 */
class Cell {
	
	Cell() {}
	
	/**
	 * The player in the cell. Currently we're limited to one player per cell.
	 */
	Player player;
	/**
	 * The list of objects in the cell, mapped by object name. The names must be
	 * unique.
	 */
	ConcurrentHashMap<String, CellObject> cellObjects = new ConcurrentHashMap<String, CellObject>();
	/**
	 * An iterator reference so we don't have to create it each time.
	 */
	Iterator<CellObject> iter;
	
	Player getPlayer() {
		return this.player;
	}
	
	void setPlayer(Player player) {
		this.player = player;
	}
	
	/**
	 * @return true if there are no cell objects with the block property
	 * 
	 * Checks to see if the cell has no blocking cell objects.
	 */
	boolean enterable() {
		// Check to see if any contained objects have the block property
		if (cellObjects.size() > 0) {
			this.iter = cellObjects.values().iterator();
			while (this.iter.hasNext()) {
				CellObject cellObject = this.iter.next();
				if (cellObject.getBooleanProperty(Names.kPropertyBlock)) {
					return false;
				}
			}
		}
		return true;
	}
	
	/**
	 * @param cellObject the object to add
	 * 
	 * Adds a cell object to the object list.
	 */
	void addCellObject(CellObject cellObject) {
		cellObjects.put(cellObject.getName(), cellObject);
	}
	
	/**
	 * @param name the property to look for
	 * @return a list of cell objects that have the specified property
	 * 
	 * Returns all objects in the cell with the specified property.
	 * The returned list is never null but could be length zero.
	 */
	ArrayList<CellObject> getAllWithProperty(String name) {
		ArrayList<CellObject> list = new ArrayList<CellObject>();
		this.iter = cellObjects.values().iterator();
		CellObject cellObject;
		while (this.iter.hasNext()) {
			cellObject = this.iter.next();
			if (cellObject.hasProperty(name)) {
				list.add(cellObject);
			}
		}
		return list;
	}
	
	/**
	 * @param name the object name
	 * @return the object or null if none
	 * 
	 * Returns the object by name. Very fast.
	 */
	CellObject getObject(String name) {
		return cellObjects.get(name);
	}
	
	/**
	 * @param name the object name
	 * @return true if the object exists in the cell
	 * 
	 * Check to see if the object with the specified name is in the cell.
	 */
	boolean hasObject(String name) {
		return cellObjects.containsKey(name);
	}
	
	/**
	 * @param name the object name
	 * @return the removed object or null if it didn't exist
	 * 
	 * If the specified object exists in the cell, it is removed and returned.
	 * Null is returned if the object isn't in the cell.
	 */
	CellObject removeObject(String name) {
		return cellObjects.remove(name);
	}

	// for sound algorithm
	int distance = -1;
	java.awt.Point parent;
}
