package edu.umich.soar.room.map;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentSkipListSet;


class SetCell implements Cell {
	private final List<Robot> players = new ArrayList<Robot>();
	private final Set<CellObject> cellObjects = new ConcurrentSkipListSet<CellObject>();

	// Weakly consistent, many races below. This is acceptable: used only for
	// rendering.
	private boolean modified = true;

	private final int[] location;

	protected SetCell(int[] location) {
		this.location = new int[] { location[0], location[1] };
	}

	@Override
	public int[] getLocation() {
		return new int[] { location[0], location[1] };
	}

	@Override
	public boolean isModified() {
		return modified;
	}

	@Override
	public void setModified(boolean value) {
		modified = value;
	}

	@Override
	public Robot getFirstPlayer() {
		synchronized (players) {
			return players.size() > 0 ? players.get(0) : null;
		}
	}

	@Override
	public List<Robot> getAllPlayers() {
		synchronized (players) {
			return new ArrayList<Robot>(players);
		}
	}

	@Override
	public void addPlayer(Robot player) {
		if (player == null) {
			throw new NullPointerException();
		}
		synchronized (players) {
			modified = players.add(player) || modified;
		}
	}

	@Override
	public void removePlayer(Robot player) {
		synchronized (players) {
			modified = players.remove(player) || modified;
		}
	}

	@Override
	public void clearPlayers() {
		synchronized (players) {
			modified = !players.isEmpty() || modified;
			players.clear();
		}
	}

	@Override
	public boolean hasPlayers() {
		synchronized (players) {
			return !players.isEmpty();
		}
	}

	@Override
	public void addObject(CellObject object) {
		if (cellObjects.add(object)) {
			modified = true;
			object.setCell(this);
			Cells.fireAddedCallbacks(object);
		}
	}

	@Override
	public Set<CellObject> getAllObjects() {
		return new HashSet<CellObject>(cellObjects);
	}

	@Override
	public Set<CellObject> removeAllObjects() {
		Set<CellObject> removed = new HashSet<CellObject>(cellObjects);
		cellObjects.clear();

		// isEmpty not called if already modified
		modified = modified || !removed.isEmpty();

		for (CellObject object : removed) {
			object.setCell(null);
			Cells.fireRemovedCallbacks(object);
		}
		return removed;
	}

	@Override
	public Set<CellObject> getAllObjectsWithProperty(String property) {
		Set<CellObject> ret = new HashSet<CellObject>();
		for (CellObject object : cellObjects) {
			if (object.hasProperty(property)) {
				ret.add(object);
			}
		}
		return ret;
	}

	@Override
	public CellObject getFirstObjectWithProperty(String property) {
		for (CellObject object : cellObjects) {
			if (object.hasProperty(property)) {
				return object;
			}
		}
		return null;
	}

	@Override
	public boolean hasObjectWithProperty(String property) {
		if (property == null) {
			throw new NullPointerException();
		}
		return getFirstObjectWithProperty(property) != null;
	}

	@Override
	public Set<CellObject> removeAllObjectsByProperty(String property) {
		Set<CellObject> ret = new HashSet<CellObject>();
		Iterator<CellObject> iter = cellObjects.iterator();
		while (iter.hasNext()) {
			CellObject object = iter.next();
			if (object.hasProperty(property)) {
				ret.add(object);
				iter.remove();
				modified = true;
				object.setCell(null);
				Cells.fireRemovedCallbacks(object);
			}
		}
		return ret;
	}

	@Override
	public boolean removeObject(CellObject object) {
		if (cellObjects.remove(object)) {
			modified = true;
			object.setCell(null);
			Cells.fireRemovedCallbacks(object);
			return true;
		}
		return false;
	}

	@Override
	public boolean hasObject(CellObject object) {
		if (object == null) {
			throw new NullPointerException();
		}
		return cellObjects.contains(object);
	}

}
