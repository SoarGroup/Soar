package soar2d.world;

import java.util.*;

import soar2d.Names;
import soar2d.World;
import soar2d.player.Player;

/**
 * @author voigtjr
 *
 * A cell in the grid based world.
 */
public class Cell {
	
	/**
	 * The player in the cell. Currently we're limited to one player per cell.
	 */
	private Player player;
	/**
	 * The list of objects in the cell, mapped by object name. The names must be
	 * unique.
	 */
	private HashMap<String, CellObject> cellObjects = new HashMap<String, CellObject>();
	/**
	 * An iterator reference so we don't have to create it each time.
	 */
	private Iterator<CellObject> iter;
	
	public Player getPlayer() {
		return this.player;
	}
	
	public void setPlayer(Player player) {
		this.player = player;
	}
	
	/**
	 * @return true if there are no cell objects with the block property
	 * 
	 * Checks to see if the cell has no blocking cell objects.
	 */
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
	
	/**
	 * @param cellObject the object to add
	 * 
	 * Adds a cell object to the object list.
	 */
	public void addCellObject(CellObject cellObject) {
		assert !cellObjects.containsKey(cellObject.getName());
		cellObjects.put(cellObject.getName(), cellObject);
	}
	
	/**
	 * @param name the property to look for
	 * @return a list of cell objects that have the specified property
	 * 
	 * Returns all objects in the cell with the specified property.
	 * The returned list is never null but could be length zero.
	 */
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
	
	/**
	 * @param name the property name
	 * 
	 * Removes all objects in the cell with the specified property.
	 */
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
	
	/**
	 * @param name the object name
	 * @return the object or null if none
	 * 
	 * Returns the object by name. Very fast.
	 */
	public CellObject getObject(String name) {
		return cellObjects.get(name);
	}
	
	/**
	 * @param name the object name
	 * @return true if the object exists in the cell
	 * 
	 * Check to see if the object with the specified name is in the cell.
	 */
	public boolean hasObject(String name) {
		return cellObjects.containsKey(name);
	}
	
	/**
	 * @param name the object name
	 * @return the removed object or null if it didn't exist
	 * 
	 * If the specified object exists in the cell, it is removed and returned.
	 * Null is returned if the object isn't in the cell.
	 */
	public CellObject removeObject(String name) {
		return cellObjects.remove(name);
	}
	
	/**
	 * @param world the world
	 * @param location the cell's location
	 * 
	 * Called when updatable objects should update their state.
	 */
	public void update(World world, java.awt.Point location) {
		this.iter = cellObjects.values().iterator();
		CellObject object;
		while (this.iter.hasNext()) {
			object = this.iter.next();
			if (object.updatable) {
				if (object.update(world, location)) {
					// remove the object
					removeObject(object.getName());
					
					// redraw this cell
					if (!hasObject(Names.kRedraw)) {
						addCellObject(new CellObject(Names.kRedraw, false, true));
					}
				}
			}
		}
	}
}
