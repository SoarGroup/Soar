package soar2d.map;

import java.awt.Point;
import java.util.Iterator;

import soar2d.configuration.Configuration;
import soar2d.world.TankSoarWorld;

public class KitchenMap extends GridMap {

	public KitchenMap(Configuration config) {
		super(config);
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

	@Override
	public boolean isAvailable(Point location) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	void removalStateUpdate(CellObject object) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void updateObjects(TankSoarWorld tsWorld) {
		// TODO Auto-generated method stub
		
	}
}
