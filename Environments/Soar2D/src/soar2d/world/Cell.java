package soar2d.world;

import java.util.*;

import soar2d.Names;
import soar2d.World;
import soar2d.player.Player;

public class Cell {
	
	private Player player;
	private HashMap<String, CellObject> cellObjects = new HashMap<String, CellObject>();
	private Iterator<CellObject> iter;
	
	public Player getPlayer() {
		return this.player;
	}
	
	public void setPlayer(Player player) {
		this.player = player;
	}
	
	public boolean enterable() {
		// Check to see if any contained objects have the block property
		if (cellObjects.size() > 0) {
			this.iter = cellObjects.values().iterator();
			while (this.iter.hasNext()) {
				CellObject cellObject = this.iter.next();
				if (cellObject.getBooleanProperty(Names.kPropertyBlock)) {
					return false;
				}
			}
		}
		return true;
	}
	
	public void addCellObject(CellObject cellObject) {
		assert !cellObjects.containsKey(cellObject.getName());
		cellObjects.put(cellObject.getName(), cellObject);
	}
	
	public ArrayList<CellObject> getAllWithProperty(String name) {
		ArrayList<CellObject> list = new ArrayList<CellObject>();
		this.iter = cellObjects.values().iterator();
		CellObject cellObject;
		while (this.iter.hasNext()) {
			cellObject = this.iter.next();
			if (cellObject.hasProperty(name)) {
				list.add(cellObject);
			}
		}
		return list;
	}
	
	public void removeAllWithProperty(String name) {
		this.iter = cellObjects.values().iterator();
		CellObject cellObject;
		while (this.iter.hasNext()) {
			cellObject = this.iter.next();
			if (cellObject.hasProperty(name)) {
				iter.remove();
			}
		}
	}
	
	public CellObject getObject(String name) {
		return cellObjects.get(name);
	}
	
	public boolean hasObject(String name) {
		return cellObjects.containsKey(name);
	}
	
	public CellObject removeObject(String name) {
		return cellObjects.remove(name);
	}
	
	public void removeObject(CellObject cellObject) {
		this.iter = cellObjects.values().iterator();
		CellObject object;
		while (this.iter.hasNext()) {
			object = this.iter.next();
			if (cellObject.name.equals(object.name)) {
				iter.remove();
				return;
			}
		}
	}
	
	public void update(World world, java.awt.Point location) {
		this.iter = cellObjects.values().iterator();
		CellObject object;
		while (this.iter.hasNext()) {
			object = this.iter.next();
			if (object.updatable) {
				if (object.update(world, location)) {
					// remove the object
					removeObject(object);
					
					// redraw this cell
					if (!hasObject(Names.kRedraw)) {
						addCellObject(new CellObject(Names.kRedraw, false, true));
					}
				}
			}
		}
	}
}
