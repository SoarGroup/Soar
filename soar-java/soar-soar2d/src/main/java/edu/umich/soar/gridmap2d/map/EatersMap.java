package edu.umich.soar.gridmap2d.map;

import java.io.File;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import edu.umich.soar.gridmap2d.Names;


public class EatersMap implements GridMap, CellObjectObserver {
	public static EatersMap generateInstance(String mapPath, boolean unopenedBoxesTerminal, double lowProbability, double highProbability) {
		return new EatersMap(mapPath, unopenedBoxesTerminal, lowProbability, highProbability);
	}
	

	private String mapPath;
	private GridMapData data;
	private int foodCount;
	private int scoreCount;
	private Set<CellObject> unopenedBoxes;
	private boolean unopenedBoxesTerminal;
	private double lowProbability;
	private double highProbability;

	private EatersMap(String mapPath, boolean unopenedBoxesTerminal, double lowProbability, double highProbability) {
		this.mapPath = new String(mapPath);
		this.lowProbability = lowProbability;
		this.highProbability = highProbability;
		
		reset();
	}
	
	public String getCurrentMapName() {
		return GridMapUtil.getMapName(this.mapPath);
	}

	public void reset(){
		foodCount = 0;
		scoreCount = 0;
		unopenedBoxes = new HashSet<CellObject>();
		data = new GridMapData();
		GridMapUtil.loadFromConfigFile(data, mapPath, this, lowProbability, highProbability);
	}
	
	public int size() {
		return data.cells.size();
	}
	
	public Cell getCell(int[] xy) {
		return data.cells.getCell(xy);
	}

	public boolean isAvailable(int[] location) {
		Cell cell = data.cells.getCell(location);
		boolean enterable = !cell.hasAnyWithProperty(Names.kPropertyBlock);
		boolean noPlayer = !cell.hasPlayers();
		return enterable && noPlayer;
	}

	public boolean isInBounds(int[] xy) {
		return data.cells.isInBounds(xy);
	}
	
	public int[] getAvailableLocationAmortized() {
		return GridMapUtil.getAvailableLocationAmortized(this);
	}

	public void addStateUpdate(int [] location, CellObject added) {
		// Update state we keep track of specific to game type
		if (added.hasProperty(Names.kPropertyEdible)) {
			foodCount += 1;
		}
		if (added.hasProperty("apply.points")) {
			scoreCount += added.getIntProperty("apply.points", 0);
		}
	}

	public void removalStateUpdate(int [] location, CellObject removed) {
		if (unopenedBoxesTerminal) {
			if (isUnopenedBox(removed)) {
				unopenedBoxes.remove(removed);
			}
		}
		
		if (removed.hasProperty(Names.kPropertyEdible)) {
			foodCount -= 1;
		}
		if (removed.hasProperty(Names.kPropertyPoints)) {
			scoreCount -= removed.getIntProperty("apply.points", 0);
		}
	}

	public void updateObjects() {
		Set<CellObject> copy = new HashSet<CellObject>(data.updatables);
		for (CellObject cellObject : copy) {
			int [] location = data.updatablesLocations.get(cellObject);
			Cell cell = getCell(location);
			
			GridMapUtil.lingerUpdate(cellObject, cell);

			// decay
			if (cellObject.hasProperty("update.decay")) {
				int points = cellObject.getIntProperty("apply.points", 0);
				int decay = cellObject.getIntProperty("update.decay", 1);
				if (decay >= points) {
					scoreCount -= points;
					cell.removeObject(cellObject.getName());
				} else {
					scoreCount -= decay;
					points -= decay;
					cellObject.setIntProperty("apply.points", points);
				}
			}
		}

		if (unopenedBoxesTerminal) {
			Iterator<CellObject> iter = unopenedBoxes.iterator();
			while (iter.hasNext()) {
				CellObject box = iter.next();
				if (!isUnopenedBox(box)) {
					iter.remove();
				}
			}
		}
	}
	
	private boolean isUnopenedBox(CellObject object) {
		if (object.hasProperty(Names.kPropertyBox)) {
			String status = object.getProperty(Names.kPropertyStatus);
			if (status == null || !status.equals(Names.kOpen)) {
				return true;
			}
		}
		return false;
	}
	
	public int getScoreCount() {
		return scoreCount;
	}
	
	public int getFoodCount() {
		return foodCount;
	}
	
	public int getUnopenedBoxCount() {
		return unopenedBoxes.size();
	}
	
	public CellObject createObjectByName(String name) {
		return data.cellObjectManager.createObject(name);
	}

	public File getMetadataFile() {
		return data.metadataFile;
	}

	public List<CellObject> getTemplatesWithProperty(String name) {
		return data.cellObjectManager.getTemplatesWithProperty(name);
	}
}
