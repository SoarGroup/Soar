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
	
	static Cell createCell(int[] xy) {
		if (useHashCells) {
			if (useSynchronized) {
				return new CellSynchronized<HashCell>(new HashCell(xy));
			}
			return new HashCell(xy);
		} else {
			if (useSynchronized) {
				return new CellSynchronized<ListCell>(new ListCell(xy));
			}
			return new ListCell(xy);
		}
	}
}
