package edu.umich.soar.gridmap2d.map;

import java.util.List;

import edu.umich.soar.gridmap2d.players.Player;

interface Cell {
	
	Player getFirstPlayer();
	List<Player> getPlayers();
	void setPlayer(Player player);
	void addPlayer(Player player);
	void removePlayer(Player player);
	void removeAllPlayers();
	boolean hasPlayers();
	
	void addObject(CellObject cellObject);
	boolean hasObject(CellObject cellObject);
	boolean removeObject(CellObject cellObject);
	List<CellObject> removeAllObjects();
	List<CellObject> removeAllObjectsByProperty(String property);
	boolean hasAnyObjectWithProperty(String property);
	List<CellObject> getAllObjects();
	List<CellObject> getAllObjectsWithProperty(String property);
	CellObject getFirstObjectWithProperty(String property);
	
	void addObserver(CellObjectObserver observer);
	void removeObserver(CellObjectObserver observer);
	
	boolean checkAndResetRedraw();
	boolean checkRedraw();
	void forceRedraw();
}
