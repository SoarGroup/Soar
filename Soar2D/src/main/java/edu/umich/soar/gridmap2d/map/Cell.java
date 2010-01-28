package edu.umich.soar.gridmap2d.map;

import java.util.List;
import java.util.Set;

import edu.umich.soar.gridmap2d.players.Player;

public interface Cell {

	public int[] getLocation();

	public Player getFirstPlayer();

	public List<Player> getAllPlayers();

	public void addPlayer(Player player);

	public void removePlayer(Player player);

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
