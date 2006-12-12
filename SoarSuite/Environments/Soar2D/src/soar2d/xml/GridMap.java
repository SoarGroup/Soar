package soar2d.xml;

import soar2d.world.Cell;
import soar2d.world.CellObjectManager;

public class GridMap {
	int size = 0;
	Cell[][] mapCells = null;
	CellObjectManager cellObjectManager = new CellObjectManager();
	
	public int getSize() {
		return size;
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
