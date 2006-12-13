package soar2d.xml;

import soar2d.world.Cell;
import soar2d.world.CellObjectManager;

/**
 * @author voigtjr
 *
 * houses the map and associated meta-data. used for grid worlds.
 */
public class GridMap {
	/**
	 * the maps are square, this is the number of row/columns
	 */
	int size = 0;
	/**
	 * the cells
	 */
	Cell[][] mapCells = null;
	/**
	 * the types of objects on this map
	 */
	CellObjectManager cellObjectManager = new CellObjectManager();
	/**
	 * true if there is a health charger
	 */
	boolean health = false;
	/**
	 * true if there is an energy charger
	 */
	boolean energy = false;
	/**
	 * returns the number of missile packs on the map
	 */
	int missilePacks = 0;
	
	public int getSize() {
		return size;
	}
	
	public int numberMissilePacks() {
		return missilePacks;
	}
	
	public boolean hasHealthCharger() {
		return health;
	}
	
	public boolean hasEnergyCharger() {
		return energy;
	}
	
	public CellObjectManager getObjectManager() {
		return cellObjectManager;
	}
	
	public Cell getCell(java.awt.Point location) {
		assert location.x >= 0;
		assert location.y >= 0;
		assert location.x < size;
		assert location.y < size;
		return mapCells[location.y][location.x];
	}
	
	public Cell getCell(int x, int y) {
		assert x >= 0;
		assert y >= 0;
		assert x < size;
		assert y < size;
		return mapCells[y][x];
	}
	
	public void shutdown() {
		mapCells = null;
		cellObjectManager = null;
		size = 0;
	}
}
