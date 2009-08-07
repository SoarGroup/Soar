package edu.umich.soar.gridmap2d.map;

import java.util.List;

import edu.umich.soar.gridmap2d.players.Player;

class CellSynchronized<E extends Cell> implements Cell {
	final E c;
	protected CellSynchronized(E c) {
		this.c = c;
	}
	
	@Override
	public synchronized Player getFirstPlayer() {
		return c.getFirstPlayer();
	}
	
	@Override
	public synchronized void setPlayer(Player player) {
		c.setPlayer(player);
	}
	
	@Override
	public synchronized void addPlayer(Player player) {
		c.addPlayer(player);
	}
	
	@Override
	public synchronized void removePlayer(Player player) {
		c.removePlayer(player);
	}
	
	@Override
	public synchronized void removeAllPlayers() {
		c.removeAllPlayers();
	}
	
	@Override
	public synchronized boolean hasPlayers() {
		return c.hasPlayers();
	}

	@Override
	public synchronized void addObject(CellObject cellObject) {
		c.addObject(cellObject);
	}
	
	@Override
	public synchronized List<CellObject> getAllWithProperty(String property) {	
		return c.getAllWithProperty(property);
	}
	
	@Override
	public synchronized List<CellObject> getAllObjects() {	
		return c.getAllObjects();
	}
	
	@Override
	public synchronized boolean hasAnyObjectWithProperty(String property) {	
		return c.hasAnyObjectWithProperty(property);
	}
	
	@Override
	public synchronized List<CellObject> removeAllObjectsByProperty(String property) {
		return c.removeAllObjectsByProperty(property);
	}
	
	@Override
	public synchronized List<CellObject> removeAllObjects() {
		return c.removeAllObjects();
	}

	@Override
	public synchronized CellObject getObject(String name) {
		return c.getObject(name);
	}
	
	@Override
	public synchronized boolean hasObject(String name) {
		return c.hasObject(name);
	}
	
	@Override
	public synchronized CellObject removeObject(String name) {
		return c.removeObject(name);
	}

	@Override
	public void addObserver(CellObjectObserver observer) {
		c.addObserver(observer);
	}

	@Override
	public boolean checkAndResetRedraw() {
		return c.checkAndResetRedraw();
	}

	@Override
	public boolean checkRedraw() {
		return c.checkRedraw();
	}

	@Override
	public void forceRedraw() {
		c.forceRedraw();
	}
}
