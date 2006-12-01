package soar2d.world;

import java.util.*;

import soar2d.Entity;
import soar2d.Names;

public class Cell {
	
	private Entity entity;
	private ArrayList<CellObject> cellObjects = new ArrayList<CellObject>();
	private ListIterator<CellObject> iter;
	
	public Entity getEntity() {
		return this.entity;
	}
	
	public void setEntity(Entity entity) {
		this.entity = entity;
	}
	
	public boolean enterable() {
		// Check to see if any contained objects have the block property
		if (cellObjects.size() > 0) {
			this.iter = cellObjects.listIterator();
			if (this.iter.hasNext()) {
				CellObject cellObject = this.iter.next();
				if (cellObject.getBooleanProperty(Names.kPropertyBlock)) {
					return false;
				}
			}
		}
		return true;
	}
	
	public void addCellObject(CellObject cellObject) {
		assert cellObjects.contains(cellObject.getName()) == false;
		cellObjects.add(cellObject);
	}
	
	public ArrayList<CellObject> getAllWithProperty(String name) {
		ArrayList<CellObject> list = new ArrayList<CellObject>();
		this.iter = cellObjects.listIterator();
		if (this.iter.hasNext()) {
			CellObject cellObject = this.iter.next();
			if (cellObject.hasProperty(name)) {
				list.add(cellObject);
			}
		}
		return list;
	}
	
	public void removeAllWithProperty(String name) {
		this.iter = cellObjects.listIterator();
		if (this.iter.hasNext()) {
			CellObject cellObject = this.iter.next();
			if (cellObject.hasProperty(name)) {
				iter.remove();
			}
		}
	}
	
	public void removeObject(CellObject cellObject) {
		this.iter = cellObjects.listIterator();
		if (this.iter.hasNext()) {
			CellObject object = this.iter.next();
			if (cellObject.name.equals(object.name)) {
				iter.remove();
				return;
			}
		}
	}
}
