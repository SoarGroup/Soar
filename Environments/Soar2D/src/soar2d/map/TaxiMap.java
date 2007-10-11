package soar2d.map;

import java.awt.Point;
import java.util.Iterator;

import soar2d.Simulation;
import soar2d.Soar2D;
import soar2d.configuration.Configuration;
import soar2d.world.TankSoarWorld;

public class TaxiMap extends GridMap {

	// this should be a property of the player, but this is easier
	int fuel;
	
	public TaxiMap(Configuration config) {
		super(config);

		fuel = Simulation.random.nextInt(8) + 5; // 5-12
	}
	
	public void consumeFuel() {
		fuel -= 1;
	}

	@Override
	public void addObjectToCell(Point location, CellObject object) {
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
		
		cell.addCellObject(object);
		setRedraw(cell);
	}
	
	@Override
	public boolean isAvailable(Point location) {
		Cell cell = getCell(location);
		boolean destination = cell.getAllWithProperty("destination").size() > 0;
		boolean fuel = cell.hasObject("fuel");
		boolean noPlayer = cell.getPlayer() == null;
		return !destination && !fuel && noPlayer;
	}

	@Override
	void removalStateUpdate(CellObject object) {
	}

	@Override
	public void updateObjects(TankSoarWorld tsWorld) {
		
	}

	public boolean isPassengerDelivered() {
		// TODO Auto-generated method stub
		return false;
	}

	public boolean isFuelNegative() {
		return fuel < 0;
	}
}
