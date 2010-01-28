package edu.umich.soar.gridmap2d.map;

import java.util.List;

public interface GridMap {

	public void reset();

	public int size();

	public boolean isAvailable(int[] xy);

	public int[] getAvailableLocationAmortized();

	public boolean isInBounds(int[] xy);

	public CellObject createObjectByName(String name);

	public List<CellObject> getTemplatesWithProperty(String name);

	public String getCurrentMapName();
	
	public Cell getCell(int[] xy);

}
