package soar2d.map;

import java.util.ArrayList;
import java.util.Iterator;

import soar2d.players.Player;

/**
 * @author voigtjr
 *
 * A cell in the grid based world.
 */
class Cell {
	
	Cell() {}
	
	boolean draw = true;
	boolean resetRedraw() {
		boolean temp = draw;
		draw = false;
		return temp;
	}
	boolean checkRedraw() {
		return draw;
	}
	void forceRedraw() {
		draw = true;
	}
	
	/** The player in the cell. Currently we're limited to one player per cell. */
	Player player;
	
	private ArrayList<CellObject> cellObjects = new ArrayList<CellObject>();
	
	Player getPlayer() {
		return this.player;
	}
	
	void setPlayer(Player player) {
		draw = true;
		this.player = player;
	}
	
	/**
	 * @param cellObject the object to add
	 * 
	 * Adds a cell object to the object list.
	 */
	synchronized void addCellObject(CellObject cellObject) {
		draw = true;
		cellObjects.add(cellObject);
	}
	
	/**
	 * @param name the property to look for
	 * @return a list of cell objects that have the specified property
	 * 
	 * Returns all objects in the cell with the specified property.
	 * The returned list is never null but could be length zero.
	 */
	synchronized ArrayList<CellObject> getAllWithProperty(String name) {	
		ArrayList<CellObject> ret = null;
		for (CellObject object : cellObjects) {
			if (object.hasProperty(name)) {
				if (ret == null) {
					ret = new ArrayList<CellObject>(1);
				}
				ret.add(object);
			}
		}
		return ret;
	}
	
	synchronized ArrayList<CellObject> getAll() {	
		return new ArrayList<CellObject>(cellObjects);
	}
	
	synchronized boolean hasAnyWithProperty(String name) {	
		for (CellObject object : cellObjects) {
			if (object.hasProperty(name)) {
				return true;
			}
		}
		return false;
	}
	
	synchronized public ArrayList<CellObject> removeAllByProperty(String name) {
		draw = true;
		ArrayList<CellObject> ret = null;
		Iterator<CellObject> iter = cellObjects.iterator();
		while(iter.hasNext()) {
			CellObject object = iter.next();
			if (object.hasProperty(name)) {
				if (ret == null) {
					ret = new ArrayList<CellObject>(1);
				}
				ret.add(object);
				iter.remove();
			}
		}
		return ret;
	}
	
	synchronized public ArrayList<CellObject> removeAll() {
		draw = true;
		ArrayList<CellObject> ret = cellObjects;
		cellObjects = new ArrayList<CellObject>();
		return ret;
	}

	/**
	 * @param name the object name
	 * @return the object or null if none
	 * 
	 * Returns the object by name.
	 */
	synchronized CellObject getObject(String name) {
		for (CellObject object : cellObjects) {
			if (object.getName().equals(name)) {
				return object;
			}
		}
		return null;
	}
	
	/**
	 * @param name the object name
	 * @return true if the object exists in the cell
	 * 
	 * Check to see if the object with the specified name is in the cell.
	 */
	synchronized boolean hasObject(String name) {
		for (CellObject object : cellObjects) {
			if (object.getName().equals(name)) {
				return true;
			}
		}
		return false;
	}
	
	/**
	 * @param name the object name
	 * @return the removed object or null if it didn't exist
	 * 
	 * If the specified object exists in the cell, it is removed and returned.
	 * Null is returned if the object isn't in the cell.
	 */
	synchronized CellObject removeObject(String name) {
		draw = true;
		Iterator<CellObject> iter = cellObjects.iterator();
		while(iter.hasNext()) {
			CellObject object = iter.next();
			if (object.getName().equals(name)) {
				iter.remove();
				return object;
			}
		}
		return null;
	}

	// for sound algorithm
	int distance = -1;
	int [] parent;
}
