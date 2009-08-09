package edu.umich.soar.gridmap2d.map;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import edu.umich.soar.gridmap2d.players.Player;

class ListCell implements Cell {
	private final List<CellObjectObserver> observers = new ArrayList<CellObjectObserver>();
	private final List<Player> players = new ArrayList<Player>();
	private final List<CellObject> cellObjects = new ArrayList<CellObject>();
	private boolean draw = true;

	protected ListCell() {
	}
	
	@Override
	public void addObserver(CellObjectObserver observer) {
		observers.add(observer);
	}
	
	@Override
	public void removeObserver(CellObjectObserver observer) {
		observers.remove(observer);
	}

	@Override
	public boolean checkAndResetRedraw() {
		boolean temp = draw;
		draw = false;
		return temp;
	}
	
	@Override
	public boolean checkRedraw() {
		return draw;
	}
	
	@Override
	public void forceRedraw() {
		draw = true;
	}
	
	@Override
	public Player getFirstPlayer() {
		return players.size() > 0 ? players.get(0) : null;
	}
	
	@Override
	public void setPlayer(Player player) {
		if (player == null) {
			removeAllPlayers();
		} else {
			assert players.isEmpty();
			draw = players.add(player) || draw;
		}
	}
	
	@Override
	public void addPlayer(Player player) {
		draw = players.add(player) || draw;
	}
	
	@Override
	public void removePlayer(Player player) {
		draw = players.remove(player) || draw;
	}
	
	@Override
	public void removeAllPlayers() {
		draw = !players.isEmpty() || draw;
		players.clear();
	}
	
	@Override
	public boolean hasPlayers() {
		return !players.isEmpty();
	}
	
	/**
	 * @param cellObject
	 * @throws NullPointerException If cellObject is null 
	 */
	@Override
	public void addObject(CellObject cellObject) {
		if (cellObject == null) {
			throw new NullPointerException("cellObject null");
		}
		Iterator<CellObject> iter = cellObjects.iterator();
		while(iter.hasNext()) {
			CellObject object = iter.next();
			if (object == cellObject) {
				return;
			}
		}
		
		draw = true;
		cellObjects.add(cellObject);
		for (CellObjectObserver observer : observers) {
			observer.addStateUpdate(cellObject);
		}
	}
	
	@Override
	public List<CellObject> getAllObjects() {	
		return new ArrayList<CellObject>(cellObjects);
	}
	
	@Override
	public List<CellObject> removeAllObjects() {
		draw = !cellObjects.isEmpty();
		List<CellObject> removed = getAllObjects();
		cellObjects.clear();

		for (CellObject obj : removed) {
			for (CellObjectObserver observer : observers) {
				observer.removalStateUpdate(obj);
			}
		}
		
		return removed;
	}

	/**
	 * @param property the property to look for
	 * @return a list of cell objects that have the specified property
	 * @throws NullPointerException If property is null
	 */
	@Override
	public List<CellObject> getAllObjectsWithProperty(String property) {	
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
	
	@Override
	public CellObject getFirstObjectWithProperty(String property) {
		if (property == null) {
			throw new NullPointerException("property is null");
		}
		
		for (CellObject object : cellObjects) {
			if (object.hasProperty(property)) {
				return object;
			}
		}
		return null;
	}

	@Override
	public boolean hasAnyObjectWithProperty(String property) {	
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
	@Override
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
				observer.removalStateUpdate(cellObject);
			}
		}
		return ret;
	}

	@Override
	public boolean removeObject(CellObject cellObject) {
		if (cellObjects.remove(cellObject)) {
			draw = true;
			for (CellObjectObserver observer : observers) {
				observer.removalStateUpdate(cellObject);
			}
			return true;
		}
		return false;
	}

	@Override
	public boolean hasObject(CellObject cellObject) {
		return cellObjects.contains(cellObject);
	}

}
