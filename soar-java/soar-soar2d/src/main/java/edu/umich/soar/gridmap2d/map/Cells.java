package edu.umich.soar.gridmap2d.map;

public class Cells {
	private static boolean useSynchronized = false;
	private static boolean useHashCells = false;

	public static void setUseSynchronized(boolean useSynchronized) {
		Cells.useSynchronized = useSynchronized;
	}
	
	public static void setUseHashCells(boolean useHashCells) {
		Cells.useHashCells = useHashCells;
	}
	
	static Cell createCell() {
		if (useHashCells) {
			if (useSynchronized) {
				return new CellSynchronized<HashCell>(new HashCell());
			}
			return new HashCell();
		} else {
			if (useSynchronized) {
				return new CellSynchronized<ListCell>(new ListCell());
			}
			return new ListCell();
		}
	}
}
