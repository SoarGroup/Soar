package edu.umich.soar.gridmap2d.map;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.players.Player;


class HashCell implements Cell {
	private static Logger logger = Logger.getLogger(ListCell.class);
	
	private final int [] location;
	private final List<CellObjectObserver> observers = new ArrayList<CellObjectObserver>();
	private final List<Player> players = new ArrayList<Player>();
	private final Map<String, CellObject> cellObjects = new HashMap<String, CellObject>();
	private boolean draw = true;

	protected HashCell(int[] xy) {
		this.location = Arrays.copyOf(xy, xy.length);
	}
	
	@Override
	public int[] getLocation() {
		return Arrays.copyOf(location, location.length);
	}
	
	@Override
	public void addObserver(CellObjectObserver observer) {
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
		CellObject old = cellObjects.put(cellObject.getName(), cellObject);
		if (old != null) {
			logger.trace("Replacing existing " + old.getName() + " with new one.");
			for (CellObjectObserver observer : observers) {
				observer.removalStateUpdate(getLocation(), old);
			}
		}
		for (CellObjectObserver observer : observers) {
			observer.addStateUpdate(getLocation(), cellObject);
		}
	}
	
	public List<CellObject> getAll() {	
		return new ArrayList<CellObject>(cellObjects.values());
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
		return cellObjects.get(name);
	}
	
	/**
	 * @param name the object name
	 * @return true if the object exists in the cell
	 * 
	 * Check to see if the object with the specified name is in the cell.
	 */
	public boolean hasObject(String name) {
		return cellObjects.containsKey(name);
	}
	
	/**
	 * @param name the object name
	 * @return the removed object or null if it didn't exist
	 * 
	 * If the specified object exists in the cell, it is removed and returned.
	 * Null is returned if the object isn't in the cell.
	 */
	public CellObject removeObject(String name) {
		CellObject removed = cellObjects.remove(name);
		if (removed != null) {
			for (CellObjectObserver observer : observers) {
				observer.removalStateUpdate(getLocation(), removed);
			}
			draw = true;
			return removed;
		}
		
		logger.trace("removeObject didn't find object to remove: " + name);
		return removed;
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
		for (CellObject object : cellObjects.values()) {
			if (object.hasProperty(property)) {
				ret.add(object);
			}
		}
		return ret;
	}
	
	public boolean hasAnyWithProperty(String property) {	
		for (CellObject object : cellObjects.values()) {
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
		Iterator<Entry<String, CellObject>> iter = cellObjects.entrySet().iterator();
		while(iter.hasNext()) {
			Entry<String, CellObject> entry = iter.next();
			CellObject object = entry.getValue();
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
