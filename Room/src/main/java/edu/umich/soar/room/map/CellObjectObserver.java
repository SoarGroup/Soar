package edu.umich.soar.room.map;

interface CellObjectObserver {
	void addStateUpdate(CellObject object);

	void removalStateUpdate(CellObject object);
}
