package edu.umich.soar.room.map;

class GridMapCells {
	private final Cell[][] cells;

	GridMapCells(int size, CellObjectObserver[] observers) {
		assert size > 0;

		cells = new Cell[size][];
		int[] xy = new int[2];
		for (xy[0] = 0; xy[0] < size; ++xy[0]) {
			cells[xy[0]] = new Cell[size];
			for (xy[1] = 0; xy[1] < size; ++xy[1]) {
				Cell newCell = Cells.createCell(xy);
				setCell(xy, newCell);
			}
		}

		for (CellObjectObserver observer : observers) {
			Cells.addObserver(observer);
		}
	}

	void setCell(int[] xy, Cell cell) {
		cells[xy[0]][xy[1]] = cell;
	}

	Cell getCell(int[] xy) {
		assert xy != null;

		return cells[xy[0]][xy[1]];
	}

	int size() {
		return cells.length;
	}

	boolean isInBounds(int[] location) {
		assert location != null;

		return (location[0] >= 0) && (location[1] >= 0)
				&& (location[0] < size()) && (location[1] < size());
	}

}
