package soar2d.map;

import java.awt.Point;

import soar2d.world.TankSoarWorld;

public interface IGridMap {

	public void postCell(boolean background, GridMap map, Point location);
	public void addObjectToCell(GridMap map, CellObject object);
	public int getLocationId(GridMap map, Point location);
	public boolean isAvailable(GridMap map, Point location);
	public boolean objectIsBackground(CellObject cellObject);
	public void removalStateUpdate(GridMap map, CellObject object);
	public CellObject createExplosion(GridMap map);
	public void updateObjects(GridMap map, TankSoarWorld tsWorld);
}
