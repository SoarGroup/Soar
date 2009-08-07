package edu.umich.soar.gridmap2d.map;

import java.util.List;

import edu.umich.soar.gridmap2d.players.Player;

interface Cell {
	Player getFirstPlayer();
	void setPlayer(Player player);
	void addPlayer(Player player);
	void removePlayer(Player player);
	void removeAllPlayers();
	boolean hasPlayers();
	List<CellObject> removeAllObjects();
	CellObject getObject(String name);
	boolean hasObject(String name);
	CellObject removeObject(String name);
	List<CellObject> removeAllObjectsByProperty(String property);
	boolean hasAnyObjectWithProperty(String property);
	List<CellObject> getAllObjects();
	List<CellObject> getAllWithProperty(String property);
	void addObject(CellObject cellObject);
	void addObserver(CellObjectObserver observer);
	boolean checkAndResetRedraw();
	boolean checkRedraw();
	void forceRedraw();
}
