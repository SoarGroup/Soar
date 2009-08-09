package edu.umich.soar.gridmap2d.map;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import edu.umich.soar.gridmap2d.players.Player;


class HashCell implements Cell {
	private final List<CellObjectObserver> observers = new ArrayList<CellObjectObserver>();
	private final List<Player> players = new ArrayList<Player>();
	private final Set<CellObject> objects = new HashSet<CellObject>();
	private final Map<String, List<CellObject>> byProperty = new HashMap<String, List<CellObject>>();
	private boolean draw = true;

	protected HashCell() {
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
	public List<Player> getPlayers() {
		return new ArrayList<Player>(players);
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
		
		if (objects.add(cellObject)) {
			draw = true;

			for (String key : cellObject.getPropertyList()) {
				List<CellObject> objs = byProperty.get(key);
				if (objs == null) {
					objs = new ArrayList<CellObject>(3);
				}
				objs.add(cellObject);
				byProperty.put(key, objs);
			}
			
			fireAddedCallbacks(cellObject);
		}
	}
	
	@Override
	public List<CellObject> getAllObjects() {	
		return new ArrayList<CellObject>(objects);
	}
	
	@Override
	public List<CellObject> removeAllObjects() {
		draw = !objects.isEmpty();
		List<CellObject> removed = getAllObjects();
		objects.clear();
		byProperty.clear();

		for (CellObject obj : removed) {
			fireRemovedCallbacks(obj);
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
		
		List<CellObject> ret = byProperty.get(property);
		if (ret == null) {
			return new ArrayList<CellObject>(0);
		}
		return new ArrayList<CellObject>(ret);
	}
	
	@Override
	public CellObject getFirstObjectWithProperty(String property) {
		if (property == null) {
			throw new NullPointerException("property is null");
		}
		
		List<CellObject> ret = byProperty.get(property);
		if (ret == null) {
			return null;
		}
		
		assert ret.size() > 0;
		return ret.get(0);
	}

	@Override
	public boolean hasAnyObjectWithProperty(String property) {	
		return byProperty.containsKey(property);
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
		
		List<CellObject> temp = byProperty.get(property);
		if (temp == null) {
			return new ArrayList<CellObject>(0);
		}
		
		// must copy this or face concurrent modification exception
		List<CellObject> removed = new ArrayList<CellObject>(temp);

		draw = true;
		objects.removeAll(removed);

		// This seems expensive
		for (CellObject object : removed) {
			unmapProperties(object);
		}
		
		for (CellObject object : removed) {
			fireRemovedCallbacks(object);
		}
		
		return removed;
	}
	
	private void unmapProperties(CellObject object) {
		for (String property : object.getPropertyList()) {
			List<CellObject> objs = byProperty.remove(property);
			objs.remove(object);
			if (!objs.isEmpty()) {
				byProperty.put(property, objs);
			}
		}
	}

	@Override
	public boolean removeObject(CellObject object) {
		if (objects.remove(object)) {
			draw = true;
			unmapProperties(object);
			fireRemovedCallbacks(object);
			return true;
		}
		return false;
	}

	@Override
	public boolean hasObject(CellObject cellObject) {
		return objects.contains(cellObject);
	}
	
	private void fireAddedCallbacks(CellObject object) {
		for (CellObjectObserver observer : observers) {
			observer.addStateUpdate(object);
		}
	}

	private void fireRemovedCallbacks(CellObject object) {
		for (CellObjectObserver observer : observers) {
			observer.removalStateUpdate(object);
		}
	}

}
