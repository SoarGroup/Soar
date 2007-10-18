package soar2d.map;

import java.awt.Point;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
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
		
		fuel = Simulation.random.nextInt(1 + Soar2D.xConfig.getFuelStartingMaximum() - Soar2D.xConfig.getFuelStartingMinimum() ) + Soar2D.xConfig.getFuelStartingMinimum(); 
	}
	
	public void consumeFuel() {
		if (Soar2D.xConfig.getDisableFuel()) {
			logger.info("fuel consumption disabled");
		}
		logger.info("fuel: " + Integer.toString(fuel) + " -> " + Integer.toString(fuel-1));
		fuel -= 1;
	}
	
	// passenger
	String passengerDestination = null;
	public boolean passengerOnMap() {
		return passengerDestination != null;
	}
	public String getPassengerDestination() {
		return passengerDestination;
	}
	
	public Point getNewPassengerLocation() {
		Collection<Point> locations = destinationLocations.values();
		
		if (locations.size() < 1) {
			return null;
		}
		
		int pick = Simulation.random.nextInt(locations.size());
		Iterator<Point> iter = locations.iterator();
		for (int index = 0; index < pick; index++) {
			assert iter.hasNext();
			iter.next();
		}
		assert iter.hasNext();
		return iter.next();
	}

	HashSet<String> destinationColors = new HashSet<String>();
	HashMap<CellObject, Point> destinationLocations = new HashMap<CellObject, Point>();
	
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
		
		if (object.hasProperty("destination")) {
			destinationColors.add(object.getProperty("color"));
			destinationLocations.put(object, location);
		}
		
		if (passengerDestination == null) {
			if (object.hasProperty("passenger")) {
				if (object.hasProperty("passenger-destination")) {
					passengerDestination = object.getProperty("passenger-destination");
				} else {
					int pick = Simulation.random.nextInt(destinationColors.size());
					Iterator<String> iter = destinationColors.iterator();
					for (int index = 0; index < pick; index++) {
						assert iter.hasNext();
						iter.next();
					}
					assert iter.hasNext();
					passengerDestination = iter.next();
				}
				logger.info("passenger destination: " + passengerDestination);
			}
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
		if (object.hasProperty("destination")) {
			destinationColors.remove(object.getProperty("color"));
			destinationLocations.remove(object);
		}
	}

	@Override
	public void updateObjects(TankSoarWorld tsWorld) {
		
	}

	public boolean isFuelNegative() {
		return fuel < 0;
	}
	
	public int getFuel() {
		return fuel;
	}

	public boolean fillUp(Point location) {
		CellObject fuelObject = getObject(location, "fuel");
		if (fuelObject == null) {
			return false;
		}
		int maximum = Soar2D.xConfig.getFuelMaximum();
		logger.info("fuel: " + Integer.toString(fuel) + " -> " + maximum + " (fillup)");
		fuel = maximum;
		return true;
	}

	CellObject passenger = null;
	public boolean isPassengerCarried() {
		return passenger != null;
	}
	
	public boolean pickUp(Point location) {
		if (passenger != null) {
			return false;
		}
		passenger = removeObject(location, "passenger");
		if (passenger == null) {
			return false;
		}
		return true;
	}
	public boolean putDown(Point location) {
		if (passenger == null) {
			return false;
		}
		addObjectToCell(location, passenger);
		passenger = null;
		return true;
	}

	public boolean isPassengerDestination(Point location) {
		ArrayList<CellObject>destinations = getAllWithProperty(location, "destination");
		if (destinations.size() < 1) {
			return false;
		}
		assert destinations.size() == 1;
		CellObject destination = destinations.get(0);
		if (passengerDestination.equals(destination.getProperty("color"))) {
			return true;
		}
		return false;
	}

	boolean passengerDelivered = false;
	public boolean isPassengerDelivered() {
		return passengerDelivered;
	}

	public void delivered() {
		passengerDelivered = true;
	}

	public String getStringType(Point location) {
		ArrayList<CellObject> dests = this.getAllWithProperty(location, "destination");
		assert dests.size() < 2;
		if (dests.size() > 0) {
			return dests.get(0).getProperty("color");
		}
		
		if (this.getObject(location, "fuel") != null) {
			return "fuel";
		}
		
		return "normal";
	}
}
