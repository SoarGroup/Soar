package edu.umich.soar.gridmap2d.map;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.config.Config;
import edu.umich.soar.config.ConfigFile;
import edu.umich.soar.config.ParseError;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Simulation;

abstract class GridMapBase implements GridMap, CellObjectObserver {
	private static final Log logger = LogFactory.getLog(GridMapBase.class);
	
	private GridMapData data;
	private final String mapPath;

	protected GridMapBase(String mapPath) {
		this.mapPath = mapPath;
	}
	
	protected GridMapData getData() {
		return data;
	}
	
	protected String getMapPath() {
		return mapPath;
	}
	
	@Override
	public boolean isInBounds(int[] xy) {
		return data.cells.isInBounds(xy);
	}
	
	@Override
	public String getCurrentMapName() {
		// can't just use system separator, could come from config file which needs to work across systems
		int index = mapPath.lastIndexOf("/");
		if (index == -1) {
			index = mapPath.lastIndexOf("\\");
			if (index == -1) {
				return mapPath;
			}
		}
		return mapPath.substring(index + 1);
	}

	@Override
	public int size() {
		return data.cells.size();
	}

	@Override
	public int[] getAvailableLocationAmortized() {
		int size = size();
		
		// Loop in case there are no free spots, the 100 is totally arbitrary
		int [] xy = new int [2];
		for (int counter = 0; counter < 100; ++counter) {
			xy[0] = Simulation.random.nextInt(size - 2) + 1;
			xy[1] = Simulation.random.nextInt(size - 2) + 1;
			
			if (isAvailable(xy)) {
				return xy;
			}
		}
		List<int []> locations = new ArrayList<int []>();
		for (xy[0] = 0; xy[0] < size; ++xy[0]) {
			for (xy[1] = 0; xy[1] < size; ++ xy[1]) {
				if (isAvailable(xy)) {
					locations.add(xy);
				}
			}
		}
		if (locations.size() == 0) {
			return null;
		}
		return locations.get(Simulation.random.nextInt(locations.size()));
	}

	@Override
	public CellObject createObjectByName(String name) {
		return data.cellObjectManager.createObject(name);
	}

	@Override
	public List<CellObject> getTemplatesWithProperty(String name) {
		return data.cellObjectManager.getTemplatesWithProperty(name);
	}

	protected boolean reload() {
		data = new GridMapData();
		
		File mapFile = new File(mapPath);
		if (!mapFile.exists()) {
			Gridmap2D.control.errorPopUp("Map file doesn't exist: " + mapFile.getAbsolutePath());
			return false;
		}

		data.cellObjectManager = new CellObjectManager();
		
		String mapFilePath = mapFile.getAbsolutePath();
		try {
			Config mapConfig = new Config(new ConfigFile(mapFilePath));
			String objectsFileString = mapConfig.getString("objects_file");
			String mapFileDirectory = mapFilePath.substring(0, mapFilePath.lastIndexOf(File.separatorChar) + 1);
			
			Config objectsConfig = new Config(new ConfigFile(mapFileDirectory + objectsFileString));
			
			for (String id : mapConfig.getStrings("objects")) {
				CellObject template = new CellObject(objectsConfig.getChild("objects." + id));
				data.cellObjectManager.registerTemplate(template);
			}
			
			cellsConfig(mapConfig.getChild("cells"), objectsConfig);
			return true;
		} 
		catch (IOException e) {
			e.printStackTrace();
			Gridmap2D.control.errorPopUp(e.toString());
			return false;
		} 
		catch (ParseError e) {
			e.printStackTrace();
			Gridmap2D.control.errorPopUp(e.toString());
			return false;
		}
		catch (IllegalStateException e) {
			e.printStackTrace();
			Gridmap2D.control.errorPopUp(e.toString());
			return false;

		} 
		catch (IndexOutOfBoundsException e) {
			e.printStackTrace();
			Gridmap2D.control.errorPopUp(e.toString());
			return false;
		}
	}

	/**
	 * @param cellsConfig
	 * @param objectsConfig
	 * 
	 * @throws IndexOutOfBoundsException If rows do not contain enough cell data
	 * @throws IllegalArgumentException If encountered object that isn't registered
	 */
	private void cellsConfig(Config cellsConfig, Config objectsConfig) {
		data.cells = new GridMapCells(cellsConfig.requireInt("size"), new CellObjectObserver[] { data, this });
		
		data.randomWalls = cellsConfig.getBoolean("random_walls", false);
		data.randomFood = cellsConfig.getBoolean("random_food", false);
		
		// Generate map unless both are true
		if (!data.randomWalls || !data.randomFood) {
			Config rows = cellsConfig.getChild("rows");
			int[] xy = new int[2];
			for (xy[1] = 0; xy[1] < data.cells.size(); ++xy[1]) {

				String[] cellStrings = rows.getStrings(Integer.toString(xy[1]));
				if (cellStrings.length != data.cells.size()) {
					throw new IndexOutOfBoundsException("Not enough cells, row " + xy[1]);
				}
				
				for (xy[0] = 0; xy[0] < data.cells.size(); ++xy[0]) {
					String contents = cellStrings[xy[0]];
					for (String objectID : contents.split("-")) {
						String objectName = objectsConfig.getString("objects." + objectID + ".name");
						logger.trace(Arrays.toString(xy) + ": " + objectName);
						
						if (!data.cellObjectManager.hasTemplate(objectName)) {
							throw new IllegalArgumentException("object \"" + objectName + "\" does not map to a cell object");
						}
						
						CellObject cellObject = data.cellObjectManager.createObject(objectName);
						data.cells.getCell(xy).addObject(cellObject);
					}
				}
			}
		}
		
	}
	
	protected void lingerUpdate(CellObject cellObject, Cell cell) {
		if (cellObject.hasProperty("update.linger")) {
			int linger = cellObject.getProperty("update.linger", 0, Integer.class);
			linger -= 1;
			if (linger <= 0) {
				cell.removeObject(cellObject);
			} else {
				cellObject.setProperty("update.linger", linger);
			}
		}
	}
	
	@Override
	public Cell getCell(int[] xy) {
		return data.cells.getCell(xy);
	}
}
