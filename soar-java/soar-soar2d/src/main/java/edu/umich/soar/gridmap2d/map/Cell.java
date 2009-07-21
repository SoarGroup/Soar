package edu.umich.soar.gridmap2d.map;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.players.Player;


public class Cell {
	private static Logger logger = Logger.getLogger(Cell.class);

	private static boolean useSynchronized = false;
	
	public static void setUseSynchronized(boolean useSynchronized) {
		Cell.useSynchronized = useSynchronized;
	}
	
	static Cell createCell(int[] xy) {
		if (Cell.useSynchronized) {
			return new CellSynchronized(xy);
		}
		return new Cell(xy);
	}
	
	protected Cell(int[] xy) {
		this.location = edu.umich.soar.gridmap2d.Arrays.copyOf(xy, xy.length);
	}
	int [] location;
	public int[] getLocation() {
		return edu.umich.soar.gridmap2d.Arrays.copyOf(location, location.length);
	}
	
	private List<CellObjectObserver> observers = new ArrayList<CellObjectObserver>();
	void addObserver(CellObjectObserver observer) {
		observers.add(observer);
	}
	
	@Override
	public String toString() {
		return "cell" + Arrays.toString(location);
	}
	
	Cell[] neighbors = new Cell[5]; // uses Direction, which is 1-4 not 0-5
	
	private boolean draw = true;
	public boolean checkAndResetRedraw() {
		boolean temp = draw;
		draw = false;
		return temp;
	}
	public boolean checkRedraw() {
		return draw;
	}
	public void forceRedraw() {
		draw = true;
	}
	
	/** The player in the cell. Currently we're limited to one player per cell. */
	private Player player;
	
	// TODO: Compare performance between one HashMap<name, object> vs. this
	// Iteration performance is paramount
	private List<CellObject> cellObjects = new ArrayList<CellObject>();
	
	// for sound algorithm
	boolean explored = false;
	int distance = -1;
	Cell parent;
	
	public Player getPlayer() {
		return this.player;
	}
	
	public void setPlayer(Player player) {
		draw = true;
		this.player = player;
	}
	
	/** Objects keyed by name, not null name will replace existing if any */
	public void addObject(CellObject cellObject) {
		draw = true;
		if (cellObject == null) {
			throw new NullPointerException();
		}
		Iterator<CellObject> iter = cellObjects.iterator();
		while(iter.hasNext()) {
			CellObject object = iter.next();
			if (object.getName().equals(cellObject.getName())) {
				logger.trace("Replacing existing " + object.getName() + " with new one.");
				iter.remove();
				for (CellObjectObserver observer : observers) {
					observer.removalStateUpdate(location, object);
				}
				// no more iteration, removal state could change cellObjects!
				break;
			}
		}
		
		cellObjects.add(cellObject);
		for (CellObjectObserver observer : observers) {
			observer.addStateUpdate(location, cellObject);
		}
	}
	
	public List<CellObject> getAll() {	
		if (cellObjects.size() == 0) {
			return null;
		}
		return new ArrayList<CellObject>(cellObjects);
	}
	
	public List<CellObject> removeAll() {
		if (cellObjects.size() == 0) {
			return null;
		}
		draw = true;
		List<CellObject> ret = cellObjects;
		cellObjects = new ArrayList<CellObject>();
		for (CellObject cellObject : ret) {
			for (CellObjectObserver observer : observers) {
				observer.removalStateUpdate(location, cellObject);
			}
		}
		return ret;
	}

	/**
	 * @param name the object name
	 * @return the object or null if none
	 * 
	 * Returns the object by name.
	 */
	public CellObject getObject(String name) {
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
	public boolean hasObject(String name) {
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
	public CellObject removeObject(String name) {
		if (name == null) {
			throw new NullPointerException();
		}
		Iterator<CellObject> iter = cellObjects.iterator();
		while(iter.hasNext()) {
			CellObject object = iter.next();
			if (object.getName().equals(name)) {
				iter.remove();
				for (CellObjectObserver observer : observers) {
					observer.removalStateUpdate(location, object);
				}
				// no more iteration, removal state could change cellObjects!
				draw = true;
				return object;
			}
		}
		logger.trace("removeObject didn't find object to remove: " + name);
		return null;
	}

	/**
	 * @param name the property to look for
	 * @return a list of cell objects that have the specified property
	 * 
	 * Returns all objects in the cell with the specified property.
	 * The returned list is never null but could be length zero.
	 */
	public List<CellObject> getAllWithProperty(String name) {	
		List<CellObject> ret = null;
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
	
	public boolean hasAnyWithProperty(String name) {	
		for (CellObject object : cellObjects) {
			if (object.hasProperty(name)) {
				return true;
			}
		}
		return false;
	}
	
	public List<CellObject> removeAllByProperty(String name) {
		List<CellObject> ret = null;
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
		if (ret == null) {
			return null;
		}
		for (CellObject cellObject : ret) {
			// needs to be outside above loop because cellObjects could change.
			for (CellObjectObserver observer : observers) {
				observer.removalStateUpdate(location, cellObject);
			}
		}
		draw = true;
		return ret;
	}	
}
