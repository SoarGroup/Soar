package soar2d.map;

import java.awt.Point;
import java.util.Iterator;

import soar2d.world.TankSoarWorld;

public class KitchenMap implements IGridMap {

	public void addObjectToCell(GridMap map, CellObject object) {
		// TODO Auto-generated method stub
		
	}

	public int getLocationId(GridMap map, Point location) {
		// TODO Auto-generated method stub
		return -1;
	}

	public boolean isAvailable(GridMap map, Point location) {
		// TODO Auto-generated method stub
		return false;
	}

	public boolean objectIsBackground(CellObject cellObject) {
		// TODO Auto-generated method stub
		return false;
	}

	public void postCell(boolean background, GridMap map, Point location) {
		// TODO Auto-generated method stub
		
	}

	public void removalStateUpdate(GridMap map, CellObject object) {
		// TODO Auto-generated method stub
		
	}

	public CellObject createExplosion(GridMap map) {
		assert false;
		return null;
	}

	public void updateObjects(GridMap map, TankSoarWorld tsWorld) {
		if (!map.updatables.isEmpty()) {
			Iterator<CellObject> iter = map.updatables.iterator();
			
			while (iter.hasNext()) {
				CellObject cellObject = iter.next();
				java.awt.Point location = map.updatablesLocations.get(cellObject);
				assert location != null;
				
				if (cellObject.update(location)) {
					Cell cell = map.getCell(location);
					
					cellObject = cell.removeObject(cellObject.getName());
					assert cellObject != null;
					
					map.setRedraw(cell);
					
					iter.remove();
					map.updatablesLocations.remove(cellObject);
					map.removalStateUpdate(cellObject);
				}
			}
		}
	}
}
