package soar2d.map;

import java.util.HashSet;
import java.util.Iterator;

import soar2d.Names;
import soar2d.Soar2D;
import soar2d.world.TankSoarWorld;

public class EatersMap extends GridMap {

	public EatersMap() {
	}

	@Override
	public void updateObjects(TankSoarWorld tsWorld) {
		HashSet<CellObject> copy = new HashSet<CellObject>(updatables);
		for (CellObject cellObject : copy) {
			int [] location = updatablesLocations.get(cellObject);
			
			int previousScore = setPreviousScore(cellObject);

			if (cellObject.update(location)) {
				removalStateUpdate(getCell(location).removeObject(cellObject.getName()));
			}

			// decay
			if (cellObject.hasProperty(Names.kPropertyPoints)) {
				scoreCount += cellObject.getIntProperty(Names.kPropertyPoints) - previousScore;
			}
		}

		if (Soar2D.config.terminalsConfig().unopened_boxes) {
			Iterator<CellObject> iter = unopenedBoxes.iterator();
			while (iter.hasNext()) {
				CellObject box = iter.next();
				if (!isUnopenedBox(box)) {
					iter.remove();
				}
			}
		}
	}
	
	int scoreCount = 0;
	public int getScoreCount() {
		return scoreCount;
	}
	
	int foodCount = 0;
	public int getFoodCount() {
		return foodCount;
	}
	
	HashSet<CellObject> unopenedBoxes = new HashSet<CellObject>();
	public int getUnopenedBoxCount() {
		return unopenedBoxes.size();
	}
	
	@Override
	public boolean isAvailable(int [] location) {
		Cell cell = getCell(location);
		boolean enterable = !cell.hasAnyWithProperty(Names.kPropertyBlock);
		boolean noPlayer = cell.getPlayer() == null;
		return enterable && noPlayer;
	}
	
	@Override
	public void setExplosion(int [] location) {
		CellObject explosion = new CellObject(Names.kExplosion);
		explosion.addProperty(Names.kPropertyLinger, "2");
		explosion.setLingerUpdate(true);
		addObjectToCell(location, explosion);
	}
	
	private int setPreviousScore(CellObject cellObject) {
		if (cellObject.hasProperty(Names.kPropertyPoints)) {
			return cellObject.getIntProperty(Names.kPropertyPoints);
		}
		return 0;
	}
	
	@Override
	void addStateUpdate(int [] location, CellObject added) {
		super.addStateUpdate(location, added);
		// Update state we keep track of specific to game type
		if (added.hasProperty(Names.kPropertyEdible)) {
			foodCount += 1;
		}
		if (added.hasProperty(Names.kPropertyPoints)) {
			scoreCount += added.getIntProperty(Names.kPropertyPoints);
		}
	}

	@Override
	void removalStateUpdate(CellObject removed) {
		super.removalStateUpdate(removed);
		if (Soar2D.config.terminalsConfig().unopened_boxes) {
			if (isUnopenedBox(removed)) {
				unopenedBoxes.remove(removed);
			}
		}
		
		if (removed.hasProperty(Names.kPropertyEdible)) {
			foodCount -= 1;
		}
		if (removed.hasProperty(Names.kPropertyPoints)) {
			scoreCount -= removed.getIntProperty(Names.kPropertyPoints);
		}
	}
}
