package edu.umich.soar.gridmap2d.map;

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

public class Cells {
	private static final List<CellObjectObserver> observers = new CopyOnWriteArrayList<CellObjectObserver>();

	static Cell createCell(int[] location) {
		return new SetCell(location);
	}

	public static void addObserver(CellObjectObserver observer) {
		observers.add(observer);
	}

	public static void removeObserver(CellObjectObserver observer) {
		observers.remove(observer);
	}

	static void fireAddedCallbacks(CellObject object) {
		for (CellObjectObserver observer : observers) {
			observer.addStateUpdate(object);
		}
	}

	static void fireRemovedCallbacks(CellObject object) {
		for (CellObjectObserver observer : observers) {
			observer.removalStateUpdate(object);
		}
	}

	static boolean updatable(CellObject added) {
		return added.hasProperty("update.decay")
				|| added.hasProperty("update.fly-missile")
				|| added.hasProperty("update.linger");
	}
}
