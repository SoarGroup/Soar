package edu.umich.soar.gridmap2d.map;

import java.util.HashSet;
import java.util.Set;

import org.apache.log4j.Logger;

class GridMapData implements CellObjectObserver {
	private static Logger logger = Logger.getLogger(GridMapData.class);

	CellObjectManager cellObjectManager;
	GridMapCells cells;

	Set<CellObject> updatables = new HashSet<CellObject>();

	@Override
	public void addStateUpdate(CellObject added) {
		if (added.updatable()) {
			logger.trace("Adding updatable " + added.getProperty("name"));
			updatables.add(added);
		}
	}
	
	@Override
	public void removalStateUpdate(CellObject removed) {
		if (removed.updatable()) {
			logger.trace("Removing updatable " + removed.getProperty("name"));
			updatables.remove(removed);
		}
	}
	
	boolean randomWalls; // TODO: move out of here?
	boolean randomFood;
	
	CellObject rewardInfoObject; // TODO: move to CellObjectObserver code
	int positiveRewardID = 0;
	
	@Override
	public String toString() {
		StringBuilder output = new StringBuilder();
		int size = cells.size();

		int [] xy = new int [2];
		for (xy[0] = 0; xy[0] < size; ++xy[0]) {
			for (xy[1] = 0; xy[1] < size; ++ xy[1]) {
				output.append(xy[0]);
				output.append(",");
				output.append(xy[1]);
				output.append(":\n");
				
				Cell cell = cells.getCell(xy);
				for (CellObject object : cell.getAllObjects()) {
					output.append("\t");
					output.append(object);
					output.append("\n");
				}
			}
		}
		return output.toString();
	}
}

