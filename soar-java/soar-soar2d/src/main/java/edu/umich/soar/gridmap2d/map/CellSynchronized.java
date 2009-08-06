package edu.umich.soar.gridmap2d.map;

import java.util.List;

import edu.umich.soar.gridmap2d.players.Player;


class CellSynchronized extends Cell {
	protected CellSynchronized(int[] xy) {
		super(xy);
	}
	
	@Override
	public synchronized Player getFirstPlayer() {
		return super.getFirstPlayer();
	}
	
	@Override
	public synchronized void setPlayer(Player player) {
		super.setPlayer(player);
	}
	
	@Override
	public synchronized void addPlayer(Player player) {
		super.addPlayer(player);
	}
	
	@Override
	public synchronized void removePlayer(Player player) {
		super.removePlayer(player);
	}
	
	@Override
	public synchronized void removeAllPlayers() {
		super.removeAllPlayers();
	}
	
	@Override
	public synchronized boolean hasPlayers() {
		return super.hasPlayers();
	}

	@Override
	public synchronized void addObject(CellObject cellObject) {
		super.addObject(cellObject);
	}
	
	@Override
	public synchronized List<CellObject> getAllWithProperty(String name) {	
		return super.getAllWithProperty(name);
	}
	
	@Override
	public synchronized List<CellObject> getAll() {	
		return super.getAll();
	}
	
	@Override
	public synchronized boolean hasAnyWithProperty(String name) {	
		return super.hasAnyWithProperty(name);
	}
	
	@Override
	public synchronized List<CellObject> removeAllObjectsByProperty(String name) {
		return super.removeAllObjectsByProperty(name);
	}
	
	@Override
	public synchronized List<CellObject> removeAllObjects() {
		return super.removeAllObjects();
	}

	@Override
	public synchronized CellObject getObject(String name) {
		return super.getObject(name);
	}
	
	@Override
	public synchronized boolean hasObject(String name) {
		return super.hasObject(name);
	}
	
	@Override
	public synchronized CellObject removeObject(String name) {
		return super.removeObject(name);
	}
}
