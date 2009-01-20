package soar2d.map;

import java.util.Arrays;
import java.util.Iterator;

import soar2d.Soar2D;
import soar2d.world.TankSoarWorld;

public class KitchenMap extends GridMap {

	public KitchenMap() {
	}
	
	boolean haveButter = false;
	int [] spawnButter;
	
	boolean haveSugar = false;
	int [] spawnSugar;
	
	boolean haveEggs = false;
	int [] spawnEggs;
	
	boolean haveFlour = false;
	int [] spawnFlour;
	
	boolean haveCinnamon = false;
	int [] spawnCinnamon;
	
	boolean haveMolasses = false;
	int [] spawnMolasses;
	
	public void spawnBasics() {
		if (!haveButter) {
			Soar2D.logger.info("Spawning butter");
			this.addObjectToCell(spawnButter, this.cellObjectManager.createObject("butter"));
		}
		if (!haveSugar) {
			Soar2D.logger.info("Spawning sugar");
			this.addObjectToCell(spawnSugar, this.cellObjectManager.createObject("sugar"));
		}
		if (!haveEggs) {
			Soar2D.logger.info("Spawning eggs");
			this.addObjectToCell(spawnEggs, this.cellObjectManager.createObject("eggs"));
		}
		if (!haveFlour) {
			Soar2D.logger.info("Spawning flour");
			this.addObjectToCell(spawnFlour, this.cellObjectManager.createObject("flour"));
		}
		if (!haveCinnamon) {
			Soar2D.logger.info("Spawning cinnamon");
			this.addObjectToCell(spawnCinnamon, this.cellObjectManager.createObject("cinnamon"));
		}
		if (!haveMolasses) {
			Soar2D.logger.info("Spawning molasses");
			this.addObjectToCell(spawnMolasses, this.cellObjectManager.createObject("molasses"));
		}
	}

	@Override
	public void addObjectToCell(int [] location, CellObject object) {
		Cell cell = getCell(location);
		if (cell.hasObject(object.getName())) {
			CellObject old = cell.removeObject(object.getName());
			assert old != null;
			updatables.remove(old);
			updatablesLocations.remove(old);
			removalStateUpdate(old);
		}
		if (object.updatable()) {
			updatables.add(object);
			updatablesLocations.put(object, location);
		}
		
		// Update state we keep track of specific to game type
		checkBasics(location, object, true);
		
		cell.addCellObject(object);
		setRedraw(cell);
	}
	
	private void checkBasics(int [] location, CellObject object, boolean adding) {
		if (object.getName().equals("butter")) {
			assert adding ^ haveButter;
			haveButter = adding;
			if (spawnButter == null) {
				spawnButter = Arrays.copyOf(spawnButter, spawnButter.length);
			}
		}
		if (object.getName().equals("sugar")) {
			assert adding ^ haveSugar;
			haveSugar = adding;
			if (spawnSugar == null) {
				spawnSugar = Arrays.copyOf(spawnSugar, spawnSugar.length);
			}
		}
		if (object.getName().equals("eggs")) {
			assert adding ^ haveEggs;
			haveEggs = adding;
			if (spawnEggs == null) {
				spawnEggs = Arrays.copyOf(spawnEggs, spawnEggs.length);
			}
		}
		if (object.getName().equals("flour")) {
			assert adding ^ haveFlour;
			haveFlour = adding;
			if (spawnFlour == null) {
				spawnFlour = Arrays.copyOf(spawnFlour, spawnFlour.length);
			}
		}
		if (object.getName().equals("cinnamon")) {
			assert adding ^ haveCinnamon;
			haveCinnamon = adding;
			if (spawnCinnamon == null) {
				spawnCinnamon = Arrays.copyOf(spawnCinnamon, spawnCinnamon.length);
			}
		}
		if (object.getName().equals("molasses")) {
			assert adding ^ haveMolasses;
			haveMolasses = adding;
			if (spawnMolasses == null) {
				spawnMolasses = Arrays.copyOf(spawnMolasses, spawnMolasses.length);
			}
		}
	}
	
	public void updateObjects(GridMap map, TankSoarWorld tsWorld) {
		if (!map.updatables.isEmpty()) {
			Iterator<CellObject> iter = map.updatables.iterator();
			
			while (iter.hasNext()) {
				CellObject cellObject = iter.next();
				int [] location = map.updatablesLocations.get(cellObject);
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
	public boolean isAvailable(int [] location) {
		Cell cell = getCell(location);
		boolean enterable = cell.enterable();
		boolean noPlayer = cell.getPlayer() == null;
		return enterable && noPlayer;
	}

	@Override
	void removalStateUpdate(CellObject object) {
		checkBasics(null, object, false);
	}

	@Override
	public void updateObjects(TankSoarWorld tsWorld) {
		// TODO Auto-generated method stub
		
	}

	public boolean isCountertop(int [] location) {
		return getCell(location).getObject("countertop") != null;
	}
	
	public boolean isOven(int [] location) {
		return getCell(location).getObject("oven") != null;
	}
}
