package soar2d.map;

import java.util.ArrayList;

import soar2d.players.Player;

class CellSynchronized extends Cell {
	protected CellSynchronized(int[] xy) {
		super(xy);
	}
	
	@Override
	synchronized Player getPlayer() {
		return super.getPlayer();
	}
	
	@Override
	synchronized void setPlayer(Player player) {
		super.setPlayer(player);
	}
	
	@Override
	synchronized void addObject(CellObject cellObject) {
		super.addObject(cellObject);
	}
	
	@Override
	synchronized ArrayList<CellObject> getAllWithProperty(String name) {	
		return super.getAllWithProperty(name);
	}
	
	@Override
	synchronized ArrayList<CellObject> getAll() {	
		return super.getAll();
	}
	
	@Override
	synchronized boolean hasAnyWithProperty(String name) {	
		return super.hasAnyWithProperty(name);
	}
	
	@Override
	synchronized ArrayList<CellObject> removeAllByProperty(String name) {
		return super.removeAllByProperty(name);
	}
	
	@Override
	synchronized public ArrayList<CellObject> removeAll() {
		return super.removeAll();
	}

	@Override
	synchronized CellObject getObject(String name) {
		return super.getObject(name);
	}
	
	@Override
	synchronized boolean hasObject(String name) {
		return super.hasObject(name);
	}
	
	@Override
	synchronized CellObject removeObject(String name) {
		return super.removeObject(name);
	}
}
