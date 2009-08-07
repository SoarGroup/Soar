package edu.umich.soar.gridmap2d.map;

import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import edu.umich.soar.gridmap2d.Names;

public class EatersMap extends GridMapBase implements GridMap, CellObjectObserver {
	public static EatersMap generateInstance(String mapPath, boolean unopenedBoxesTerminal, double lowProbability, double highProbability) {
		return new EatersMap(mapPath, unopenedBoxesTerminal, lowProbability, highProbability);
	}

	private int foodCount;
	private int scoreCount;
	private Set<CellObject> unopenedBoxes;
	private boolean unopenedBoxesTerminal;
	private double lowProbability;
	private double highProbability;

	private EatersMap(String mapPath, boolean unopenedBoxesTerminal, double lowProbability, double highProbability) {
		super(mapPath);
		this.lowProbability = lowProbability;
		this.highProbability = highProbability;
		
		reset();
	}
	
	@Override
	public void reset(){
		foodCount = 0;
		scoreCount = 0;
		unopenedBoxes = new HashSet<CellObject>();
		super.reload(lowProbability, highProbability);
	}
	
	@Override
	public boolean isAvailable(int[] location) {
		Cell cell = getData().cells.getCell(location);
		boolean enterable = !cell.hasAnyObjectWithProperty(Names.kPropertyBlock);
		boolean noPlayer = !cell.hasPlayers();
		return enterable && noPlayer;
	}

	@Override
	public void addStateUpdate(CellObject added) {
		// Update state we keep track of specific to game type
		if (added.hasProperty(Names.kPropertyEdible)) {
			foodCount += 1;
		}
		if (added.hasProperty("apply.points")) {
			scoreCount += added.getIntProperty("apply.points", 0);
		}
	}

	@Override
	public void removalStateUpdate(CellObject removed) {
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
		Set<CellObject> copy = new HashSet<CellObject>(getData().updatables);
		for (CellObject cellObject : copy) {
			Cell cell = getData().cells.getCell(cellObject.getLocation());
			
			lingerUpdate(cellObject, cell);

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
}
