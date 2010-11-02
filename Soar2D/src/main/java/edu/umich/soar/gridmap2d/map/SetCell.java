package edu.umich.soar.gridmap2d.map;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentSkipListSet;

import edu.umich.soar.gridmap2d.players.Player;

class SetCell implements Cell {
	private final List<Player> players = new ArrayList<Player>();
	private final Set<CellObject> cellObjects = new ConcurrentSkipListSet<CellObject>();

	// Weakly consistent, many races below. This is acceptable: used only for
	// rendering.
	private boolean modified = true;

	private final int[] location;

	protected SetCell(int[] location) {
		this.location = new int[] { location[0], location[1] };
	}

	public int[] getLocation() {
		return new int[] { location[0], location[1] };
	}

	public boolean isModified() {
		return modified;
	}

	public void setModified(boolean value) {
		modified = value;
	}

	public Player getFirstPlayer() {
		synchronized (players) {
			return players.size() > 0 ? players.get(0) : null;
		}
	}

	public List<Player> getAllPlayers() {
		synchronized (players) {
			return new ArrayList<Player>(players);
		}
	}

	public void addPlayer(Player player) {
		if (player == null) {
			throw new NullPointerException();
		}
		synchronized (players) {
			modified = players.add(player) || modified;
		}
	}

	public void removePlayer(Player player) {
		synchronized (players) {
			modified = players.remove(player) || modified;
		}
	}

	public void clearPlayers() {
		synchronized (players) {
			modified = !players.isEmpty() || modified;
			players.clear();
		}
	}

	public boolean hasPlayers() {
		synchronized (players) {
			return !players.isEmpty();
		}
	}

	public void addObject(CellObject object) {
		if (cellObjects.add(object)) {
			modified = true;
			object.setCell(this);
			Cells.fireAddedCallbacks(object);
		}
	}

	public Set<CellObject> getAllObjects() {
		return new HashSet<CellObject>(cellObjects);
	}

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

	public Set<CellObject> getAllObjectsWithProperty(String property) {
		Set<CellObject> ret = new HashSet<CellObject>();
		for (CellObject object : cellObjects) {
			if (object.hasProperty(property)) {
				ret.add(object);
			}
		}
		return ret;
	}

	public CellObject getFirstObjectWithProperty(String property) {
		for (CellObject object : cellObjects) {
			if (object.hasProperty(property)) {
				return object;
			}
		}
		return null;
	}

	public boolean hasObjectWithProperty(String property) {
		if (property == null) {
			throw new NullPointerException();
		}
		return getFirstObjectWithProperty(property) != null;
	}

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

	public boolean removeObject(CellObject object) {
		if (cellObjects.remove(object)) {
			modified = true;
			object.setCell(null);
			Cells.fireRemovedCallbacks(object);
			return true;
		}
		return false;
	}

	public boolean hasObject(CellObject object) {
		if (object == null) {
			throw new NullPointerException();
		}
		return cellObjects.contains(object);
	}

}
