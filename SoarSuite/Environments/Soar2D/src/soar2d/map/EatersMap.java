package soar2d.map;

import java.awt.Point;
import java.util.Iterator;

import soar2d.Names;
import soar2d.world.TankSoarWorld;

public class EatersMap implements IGridMap {

	public void addObjectToCell(GridMap map, CellObject object) {
		if (object.hasProperty(Names.kPropertyEdible)) {
			map.foodCount += 1;
		}
		if (object.hasProperty(Names.kPropertyPoints)) {
			map.scoreCount += object.getIntProperty(Names.kPropertyPoints);
		}
	}

	public void postCell(boolean background, GridMap map, Point location) {
	}

	public int getLocationId(GridMap gridMap, Point location) {
		return -1;
	}

	public boolean isAvailable(GridMap map, Point location) {
		Cell cell = map.getCell(location);
		boolean enterable = cell.enterable();
		boolean noPlayer = cell.getPlayer() == null;
		return enterable && noPlayer;
	}

	public boolean objectIsBackground(CellObject cellObject) {
		return false;
	}

	public void removalStateUpdate(GridMap map, CellObject object) {
		if (object.hasProperty(Names.kPropertyEdible)) {
			map.foodCount -= 1;
		}
		if (object.hasProperty(Names.kPropertyPoints)) {
			map.scoreCount -= object.getIntProperty(Names.kPropertyPoints);
		}
		
	}

	public CellObject createExplosion(GridMap map) {
		CellObject explosion = new CellObject(Names.kExplosion);
		explosion.addProperty(Names.kPropertyLinger, "2");
		explosion.setLingerUpdate(true);
		return explosion;
	}

	private int setPreviousScore(GridMap map, CellObject cellObject) {
		if (cellObject.hasProperty(Names.kPropertyPoints)) {
			return cellObject.getIntProperty(Names.kPropertyPoints);
		}
		return 0;
	}

	public void updateObjects(GridMap map, TankSoarWorld tsWorld) {
		if (!map.updatables.isEmpty()) {
			Iterator<CellObject> iter = map.updatables.iterator();
			
			while (iter.hasNext()) {
				CellObject cellObject = iter.next();
				java.awt.Point location = map.updatablesLocations.get(cellObject);
				assert location != null;
				
				int previousScore = setPreviousScore(map, cellObject);

				if (cellObject.update(location)) {
					Cell cell = map.getCell(location);
					
					cellObject = cell.removeObject(cellObject.getName());
					assert cellObject != null;
					
					map.setRedraw(cell);
					
					iter.remove();
					map.updatablesLocations.remove(cellObject);
					map.removalStateUpdate(cellObject);
				}
				if (cellObject.hasProperty(Names.kPropertyPoints)) {
					map.scoreCount += cellObject.getIntProperty(Names.kPropertyPoints) - previousScore;
				}
			}
		}
		
		if (map.config.getTerminalUnopenedBoxes()) {
			Iterator<CellObject> iter = map.unopenedBoxes.iterator();
			while (iter.hasNext()) {
				CellObject box = iter.next();
				if (!map.isUnopenedBox(box)) {
					iter.remove();
				}
			}
		}
		
	}

}
