package edu.umich.soar.gridmap2d.map;

interface CellObjectObserver {
	void addStateUpdate(CellObject object);

	void removalStateUpdate(CellObject object);
}
