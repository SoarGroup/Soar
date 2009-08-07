package edu.umich.soar.gridmap2d.map;

import java.util.List;

import edu.umich.soar.gridmap2d.players.Player;

public interface GridMap {
	public void reset();
	public int size();
	public boolean isAvailable(int[] xy);
	public int[] getAvailableLocationAmortized();
	public boolean isInBounds(int[] xy);
	public CellObject createObjectByName(String name);
	public List<CellObject> getTemplatesWithProperty(String name);
	public String getCurrentMapName();
	public Player getFirstPlayer(int[] xy);
	public void setPlayer(int[] xy, Player player);
	public void addPlayer(int[] xy, Player player);
	public void removePlayer(int[] xy, Player player);
	public void removeAllPlayers(int[] xy);
	public boolean hasPlayers(int[] xy);
	public void addObserver(int[] xy, CellObjectObserver observer);
	public boolean checkAndResetRedraw(int[] xy);
	public boolean checkRedraw(int[] xy);
	public void forceRedraw(int[] xy);
	public List<CellObject> removeAllObjects(int[] xy);
	public CellObject getObject(int[] xy, String name);
	public boolean hasObject(int[] xy, String name);
	public CellObject removeObject(int[] xy, String name);
	public List<CellObject> removeAllObjectsByProperty(int[] xy, String property);
	public boolean hasAnyObjectWithProperty(int[] xy, String property);
	public List<CellObject> getAllObjects(int[] xy);
	public List<CellObject> getAllWithProperty(int[] xy, String property);
	public void addObject(int[] xy, CellObject cellObject);
}
