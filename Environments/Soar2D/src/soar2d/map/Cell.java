package soar2d.map;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;

import soar2d.players.Player;

class Cell {
	public static Cell createCell(boolean headless, int[] xy) {
		if (headless) {
			// only one thread
			return new Cell(xy);
		}
		return new CellSynchronized(xy);
	}
	
	protected Cell(int[] xy) {
		this.location = Arrays.copyOf(xy, xy.length);
	}
	int [] location;
	
	@Override
	public String toString() {
		return "cell" + Arrays.toString(location);
	}
	
	Cell[] neighbors = new Cell[5]; // uses Direction, which is 1-4 not 0-5
	
	private boolean draw = true;
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
	private Player player;
	// TODO: Compare performance between one HashMap<name, object> vs. this
	// Iteration performance is paramount
	private ArrayList<CellObject> cellObjects = new ArrayList<CellObject>();
	
	// for sound algorithm
	boolean explored = false;
	int distance = -1;
	Cell parent;
	
	Player getPlayer() {
		return this.player;
	}
	
	void setPlayer(Player player) {
		draw = true;
		this.player = player;
	}
	
	/** Objects keyed by name, not null name will replace existing if any */
	void addObject(CellObject cellObject) {
		draw = true;
		if (cellObject == null) {
			throw new NullPointerException();
		}
		removeObject(cellObject.getName());
		cellObjects.add(cellObject);
	}
	
	ArrayList<CellObject> getAll() {	
		if (cellObjects.size() == 0) {
			return null;
		}
		return new ArrayList<CellObject>(cellObjects);
	}
	
	ArrayList<CellObject> removeAll() {
		draw = true;
		if (cellObjects.size() == 0) {
			return null;
		}
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
	CellObject getObject(String name) {
		if (name == null) {
			throw new NullPointerException();
		}
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
	boolean hasObject(String name) {
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
	CellObject removeObject(String name) {
		if (name == null) {
			throw new NullPointerException();
		}
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

	/**
	 * @param name the property to look for
	 * @return a list of cell objects that have the specified property
	 * 
	 * Returns all objects in the cell with the specified property.
	 * The returned list is never null but could be length zero.
	 */
	ArrayList<CellObject> getAllWithProperty(String name) {	
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
	
	boolean hasAnyWithProperty(String name) {	
		for (CellObject object : cellObjects) {
			if (object.hasProperty(name)) {
				return true;
			}
		}
		return false;
	}
	
	ArrayList<CellObject> removeAllByProperty(String name) {
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
}
