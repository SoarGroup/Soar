package edu.umich.soar.gridmap2d.map;

import java.io.File;
import java.util.List;

public interface GridMap {
	public void reset() throws Exception;
	public int size();
	public Cell getCell(int[] xy);
	public boolean isAvailable(int[] location);
	public int[] getAvailableLocationAmortized();
	public boolean isInBounds(int[] xy);
	public CellObject createObjectByName(String name);
	public File getMetadataFile();
	public List<CellObject> getTemplatesWithProperty(String name);
	public String getCurrentMapName();
}
