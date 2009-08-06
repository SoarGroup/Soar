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
	
	final int [] location;
	private final List<CellObjectObserver> observers = new ArrayList<CellObjectObserver>();
	final Cell[] neighbors = new Cell[5]; // uses Direction, which is 1-4 not 0-5
	private boolean draw = true;
	private final List<Player> players = new ArrayList<Player>();
	private final List<CellObject> cellObjects = new ArrayList<CellObject>();
	
	// for sound algorithm
	boolean explored = false;
	int distance = -1;
	Cell parent;
	
	protected Cell(int[] xy) {
		this.location = Arrays.copyOf(xy, xy.length);
	}
	
	public int[] getLocation() {
		return Arrays.copyOf(location, location.length);
	}
	
	void addObserver(CellObjectObserver observer) {
		observers.add(observer);
	}
	
	@Override
	public String toString() {
		return "cell" + Arrays.toString(location);
	}
	
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
	
	public Player getFirstPlayer() {
		return players.size() > 0 ? players.get(0) : null;
	}
	
	public void setPlayer(Player player) {
		if (player == null) {
			removeAllPlayers();
		} else {
			assert players.isEmpty();
			draw = players.add(player) || draw;
		}
	}
	
	public void addPlayer(Player player) {
		draw = players.add(player) || draw;
	}
	
	public void removePlayer(Player player) {
		draw = players.remove(player) || draw;
	}
	
	public void removeAllPlayers() {
		draw = !players.isEmpty() || draw;
		players.clear();
	}
	
	public boolean hasPlayers() {
		return !players.isEmpty();
	}
	
	/**
	 * Objects keyed by name, not null name will replace existing if any.
	 * 
	 * @param cellObject
	 * @throws NullPointerException If cellObject is null 
	 */
	public void addObject(CellObject cellObject) {
		if (cellObject == null) {
			throw new NullPointerException("cellObject null");
		}
		draw = true;
		Iterator<CellObject> iter = cellObjects.iterator();
		while(iter.hasNext()) {
			CellObject object = iter.next();
			if (object.getName().equals(cellObject.getName())) {
				logger.trace("Replacing existing " + object.getName() + " with new one.");
				iter.remove();
				for (CellObjectObserver observer : observers) {
					observer.removalStateUpdate(getLocation(), object);
				}
				// no more iteration, removal state could change cellObjects!
				break;
			}
		}
		
		cellObjects.add(cellObject);
		for (CellObjectObserver observer : observers) {
			observer.addStateUpdate(getLocation(), cellObject);
		}
	}
	
	public List<CellObject> getAll() {	
		return new ArrayList<CellObject>(cellObjects);
	}
	
	public List<CellObject> removeAllObjects() {
		draw = !cellObjects.isEmpty();
		List<CellObject> removed = getAll();
		cellObjects.clear();

		for (CellObject obj : removed) {
			for (CellObjectObserver observer : observers) {
				observer.removalStateUpdate(getLocation(), obj);
			}
		}
		
		return removed;
	}

	/**
	 * @param name the object name
	 * @return the object or null if none
	 * 
	 * Returns the object by name.
	 */
	public CellObject getObject(String name) {
		for (CellObject obj : cellObjects) {
			if (obj.getName().equals(name)) {
				return obj;
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
		Iterator<CellObject> iter = cellObjects.iterator();
		while(iter.hasNext()) {
			CellObject object = iter.next();
			if (object.getName().equals(name)) {
				iter.remove();
				for (CellObjectObserver observer : observers) {
					observer.removalStateUpdate(getLocation(), object);
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
	 * @param property the property to look for
	 * @return a list of cell objects that have the specified property
	 * @throws NullPointerException If property is null
	 * 
	 * Returns all objects in the cell with the specified property.
	 * The returned list is never null but could be length zero.
	 */
	public List<CellObject> getAllWithProperty(String property) {	
		if (property == null) {
			throw new NullPointerException("property is null");
		}
		List<CellObject> ret = new ArrayList<CellObject>();
		for (CellObject object : cellObjects) {
			if (object.hasProperty(property)) {
				ret.add(object);
			}
		}
		return ret;
	}
	
	public boolean hasAnyWithProperty(String property) {	
		for (CellObject object : cellObjects) {
			if (object.hasProperty(property)) {
				return true;
			}
		}
		return false;
	}
	
	/**
	 * @param property
	 * @return
	 * @throws NullPointerException If property is null
	 */
	public List<CellObject> removeAllObjectsByProperty(String property) {
		if (property == null) {
			throw new NullPointerException("property is null");
		}
		List<CellObject> ret = new ArrayList<CellObject>();
		Iterator<CellObject> iter = cellObjects.iterator();
		while(iter.hasNext()) {
			CellObject object = iter.next();
			if (object.hasProperty(property)) {
				draw = true;
				ret.add(object);
				iter.remove();
			}
		}
		for (CellObject cellObject : ret) {
			// needs to be outside above loop because cellObjects could change.
			for (CellObjectObserver observer : observers) {
				observer.removalStateUpdate(getLocation(), cellObject);
			}
		}
		return ret;
	}	
}
