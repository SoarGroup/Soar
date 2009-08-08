package edu.umich.soar.gridmap2d.map;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.Direction;
import edu.umich.soar.gridmap2d.Simulation;

public class TaxiMap extends GridMapBase implements GridMap, CellObjectObserver {
	private static Logger logger = Logger.getLogger(TaxiMap.class);
	
	public static TaxiMap generateInstance(String mapPath) {
		return new TaxiMap(mapPath);
	}

	private CellObject passenger;
	private int[] passengerLocation;
	private String passengerSourceColor;
	private String passengerDestination;
	private String passengerDefaultDestination;
	private boolean passengerDelivered;

	private List<int[]> destinations = new ArrayList<int[]>();

	private TaxiMap(String mapPath) {
		super(mapPath);
		
		reset();
	}
	
	@Override
	public void reset() {
		destinations.clear();
		passengerDestination = null;
		passengerSourceColor = null;
		super.reload();
		
		passengerDelivered = false;
		passengerLocation = null;
		passenger = getData().cellObjectManager.createObject("passenger");

		// this can be null
		passengerDefaultDestination = passenger.getProperty("passenger-destination");

		int[] dest = destinations.get(Simulation.random.nextInt(destinations.size()));
		addObject(dest, passenger);
		
		setPassengerDestination();
	}

	private void setPassengerDestination() {
		if (passengerDefaultDestination != null) {
			passengerDestination = passengerDefaultDestination;
		} else {
			int[] dest = destinations.get(Simulation.random.nextInt(destinations.size()));
			passengerDestination = getDestinationName(dest);
		}
		
		logger.info("passenger destination: " + passengerDestination);
	}
	
	private String getDestinationName(int[] location) {
		List<CellObject> objects = getData().cells.getCell(location).getAllWithProperty("destination");
		if (objects.isEmpty()) {
			return null;
		}
		return objects.get(0).getProperty("name");
	}
	
	@Override
	public boolean isAvailable(int[] location) {
		Cell cell = getData().cells.getCell(location);
		boolean destination = cell.hasAnyObjectWithProperty("destination");
		boolean fuel = cell.hasAnyObjectWithProperty("fuel");
		boolean noPlayer = !cell.hasPlayers();
		return !destination && !fuel && noPlayer;
	}

	@Override
	public void addStateUpdate(CellObject added) {
		// Update state we keep track of specific to game type
		if (added.hasProperty("destination")) {
			destinations.add(added.getLocation());
		}
		
		if (added.hasProperty("passenger")) {
			passengerLocation = added.getLocation();
			if (passengerSourceColor == null) {
				List<CellObject> dests = getData().cells.getCell(passengerLocation).getAllWithProperty("destination");
				if (!dests.isEmpty()) {
					passengerSourceColor = dests.get(0).getProperty("color");
				}
			}
		}
	}
	
	public String getPassengerSourceColor() {
		if (passengerSourceColor == null) {
			return "none";
		}
		return passengerSourceColor;
	}

	@Override
	public void removalStateUpdate(CellObject removed) {
		if (removed.hasProperty("destination")) {
			Iterator<int[]> iter = destinations.iterator();
			while (iter.hasNext()) {
				int[] dest = iter.next();
				if (Arrays.equals(dest, removed.getLocation())) {
					iter.remove();
					break;
				}
			}
		}
		
		if (removed.hasProperty("passenger")) {
			this.passengerLocation = null;
		}
	}

	public boolean pickUp(int [] location) {
		if (passengerLocation == null) {
			return false;
		}
		if (Arrays.equals(passengerLocation, location)) {
			return getData().cells.getCell(location).removeObject(passenger);
		}
		return false;
	}
	
	public boolean putDown(int[] location) {
		if (passengerLocation != null) {
			return false;
		}
		
		addObject(location, passenger);
		return true;
	}
	
	public boolean isFuel(int[] location) {
		return getData().cells.getCell(location).hasAnyObjectWithProperty("fuel");
	}
	
	public boolean exitable(int[] location, Direction direction) {
		List<CellObject> walls = getData().cells.getCell(location).getAllWithProperty("block");
		if (walls.isEmpty()) {
			return true;
		}
		for (CellObject wall : walls) {
			if (direction.equals(Direction.parse(wall.getProperty("direction")))) {
				return false;
			}
		}
		return true;
	}
	
	public boolean isPassengerCarried() {
		return passengerLocation == null;
	}
	
	public boolean isCorrectPassengerDestination(int[] location) {
		if (passengerDestination.equals(getDestinationName(location))) {
			return true;
		}
		return false;
	}

	public boolean isPassengerDelivered() {
		return passengerDelivered;
	}
	
	public void deliverPassenger() {
		passengerDelivered = true;
	}
	
	public String getPassengerDestination() {
		assert passengerDestination != null;
		return passengerDestination;
	}
	
	public String getStringType(int[] location) {
		if (!this.isInBounds(location)) {
			return "none";
		}
		
		List<CellObject> dests = getData().cells.getCell(location).getAllWithProperty("destination");
		if (!dests.isEmpty()) {
			return dests.get(0).getProperty("color");
		}
		
		if (getData().cells.getCell(location).hasAnyObjectWithProperty("fuel")) {
			return "fuel";
		}

		return "normal";
	}
	
	public boolean wall(int[] from, Direction to) {
		List<CellObject> walls = getData().cells.getCell(from).getAllWithProperty("block");
		if (!walls.isEmpty()) {
			for (CellObject wall : walls) {
				if (wall.getProperty("direction").equals(to.id())) {
					return true;
				}
			}
		}
		return false;
	}
}
