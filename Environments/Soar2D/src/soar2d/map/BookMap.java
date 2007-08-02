package soar2d.map;

import java.awt.Point;
import java.util.ArrayList;
import java.util.Iterator;

import soar2d.Names;
import soar2d.configuration.Configuration.SimType;
import soar2d.world.TankSoarWorld;

public class BookMap implements IGridMap {

	public void addObjectToCell(GridMap gridMap, CellObject object) {
	}

	public int getLocationId(GridMap map, Point location) {
		assert location != null;
		assert map.config.getType() == SimType.kBook;

		ArrayList<CellObject> locationObjects = map.getAllWithProperty(location, Names.kPropertyNumber);
		assert locationObjects.size() == 1;
		return locationObjects.get(0).getIntProperty(Names.kPropertyNumber);
	}

	public void postCell(boolean background, GridMap map, Point location) {
		// TODO Auto-generated method stub
		
	}

	public boolean isAvailable(GridMap map, Point location) {
		Cell cell = map.getCell(location);
		boolean enterable = cell.enterable();
		boolean noPlayer = cell.getPlayer() == null;
		boolean noMBlock = cell.getAllWithProperty("mblock").size() <= 0;
		return enterable && noPlayer && noMBlock;
	}

	public boolean objectIsBackground(CellObject cellObject) {
		return false;
	}

	public void removalStateUpdate(GridMap map, CellObject object) {
		
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
