package edu.umich.soar.room.map;

import java.util.HashSet;
import java.util.Set;

class GridMapData implements CellObjectObserver {
	CellObjectManager cellObjectManager;
	GridMapCells cells;

	Set<CellObject> updatables = new HashSet<CellObject>();

	@Override
	public void addStateUpdate(CellObject added) {
		if (Cells.updatable(added)) {
			updatables.add(added);
		}
	}

	@Override
	public void removalStateUpdate(CellObject removed) {
		if (Cells.updatable(removed)) {
			updatables.remove(removed);
		}
	}

	@Override
	public String toString() {
		StringBuilder output = new StringBuilder();
		int size = cells.size();

		int[] xy = new int[2];
		for (xy[0] = 0; xy[0] < size; ++xy[0]) {
			for (xy[1] = 0; xy[1] < size; ++xy[1]) {
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
