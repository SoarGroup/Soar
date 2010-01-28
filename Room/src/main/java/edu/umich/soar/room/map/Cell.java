package edu.umich.soar.room.map;

import java.util.List;
import java.util.Set;


public interface Cell {

	public int[] getLocation();

	public Robot getFirstPlayer();

	public List<Robot> getAllPlayers();

	public void addPlayer(Robot player);

	public void removePlayer(Robot player);

	public void clearPlayers();

	public boolean hasPlayers();

	public void addObject(CellObject cellObject);

	public boolean hasObject(CellObject cellObject);

	public boolean removeObject(CellObject cellObject);

	public Set<CellObject> removeAllObjects();

	public Set<CellObject> removeAllObjectsByProperty(String property);

	public Set<CellObject> getAllObjects();

	public Set<CellObject> getAllObjectsWithProperty(String property);

	public CellObject getFirstObjectWithProperty(String property);

	public boolean hasObjectWithProperty(String property);

	public boolean isModified();

	public void setModified(boolean value);
}
