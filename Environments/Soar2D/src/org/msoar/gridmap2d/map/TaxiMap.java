package org.msoar.gridmap2d.map;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

import org.apache.log4j.Logger;
import org.msoar.gridmap2d.Direction;
import org.msoar.gridmap2d.Simulation;

public class TaxiMap implements GridMap, CellObjectObserver {
	private static Logger logger = Logger.getLogger(TaxiMap.class);

	private String mapPath;
	private GridMapData data;

	private CellObject passenger;
	private int[] passengerLocation;
	private String passengerSourceColor;
	private String passengerDestination;
	private String passengerDefaultDestination;
	private boolean passengerDelivered;

	private List<int[]> destinations = new ArrayList<int[]>();

	public TaxiMap(String mapPath) throws Exception {
		this.mapPath = new String(mapPath);
		
		reset();
	}
	
	public void reset() throws Exception {
		destinations.clear();
		passengerDestination = null;
		passengerSourceColor = null;
		
		data = GridMapUtil.loadFromConfigFile(mapPath, this);
		
		passengerDelivered = false;
		passengerLocation = null;
		passenger = data.cellObjectManager.createObject("passenger");

		// this can be null
		passengerDefaultDestination = passenger.getProperty("passenger-destination");

		int[] dest = destinations.get(Simulation.random.nextInt(destinations.size()));
		data.cells.getCell(dest).addObject(passenger);
		
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
		List<CellObject> objects = data.cells.getCell(location).getAllWithProperty("destination");
		if (objects == null || objects.size() == 0) {
			return null;
		}
		return objects.get(0).getName();
	}

	public int size() {
		return data.cells.size();
	}
	
	public Cell getCell(int[] xy) {
		return data.cells.getCell(xy);
	}

	public boolean isAvailable(int[] location) {
		Cell cell = data.cells.getCell(location);
		boolean destination = cell.hasAnyWithProperty("destination");
		boolean fuel = cell.hasObject("fuel");
		boolean noPlayer = cell.getPlayer() == null;
		return !destination && !fuel && noPlayer;
	}

	public boolean isInBounds(int[] xy) {
		return data.cells.isInBounds(xy);
	}
	
	public int[] getAvailableLocationAmortized() {
		return GridMapUtil.getAvailableLocationAmortized(this);
	}

	public void addStateUpdate(int [] location, CellObject added) {
		// Update state we keep track of specific to game type
		if (added.hasProperty("destination")) {
			destinations.add(location);
		}
		
		if (added.hasProperty("passenger")) {
			passengerLocation = org.msoar.gridmap2d.Arrays.copyOf(location, location.length);
			if (passengerSourceColor == null) {
				List<CellObject> dests = data.cells.getCell(passengerLocation).getAllWithProperty("destination");
				if (dests.size() != 0) {
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

	public void removalStateUpdate(int [] location, CellObject removed) {
		if (removed.hasProperty("destination")) {
			Iterator<int[]> iter = destinations.iterator();
			while (iter.hasNext()) {
				int[] dest = iter.next();
				if (Arrays.equals(dest, location)) {
					iter.remove();
					break;
				}
			}
		}
		
		if (removed.hasProperty("passenger")) {
			this.passengerLocation = null;
		}
	}

	public CellObject createObjectByName(String name) {
		return data.cellObjectManager.createObject(name);
	}

	public File getMetadataFile() {
		return data.metadataFile;
	}

	public List<CellObject> getTemplatesWithProperty(String name) {
		return data.cellObjectManager.getTemplatesWithProperty(name);
	}
	
	public String getCurrentMapName() {
		return GridMapUtil.getMapName(this.mapPath);
	}

	public boolean pickUp(int [] location) {
		if (passengerLocation == null) {
			return false;
		}
		if (Arrays.equals(passengerLocation, location)) {
			CellObject obj = data.cells.getCell(location).removeObject("passenger");
			assert passenger == obj;
			return true;
		}
		return false;
	}
	
	public boolean putDown(int[] location) {
		if (passengerLocation != null) {
			return false;
		}
		
		data.cells.getCell(location).addObject(passenger);
		return true;
	}
	
	public boolean isFuel(int[] location) {
		return data.cells.getCell(location).hasObject("fuel");
	}
	
	public boolean exitable(int[] location, Direction direction) {
		List<CellObject> walls = data.cells.getCell(location).getAllWithProperty("block");
		if (walls == null) {
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
		
		List<CellObject> dests = this.getCell(location).getAllWithProperty("destination");
		if (dests != null) {
			assert dests.size() < 2;
			if (dests.size() > 0) {
				return dests.get(0).getProperty("color");
			}
			
		}
		
		if (this.getCell(location).getObject("fuel") != null) {
			return "fuel";
		}

		return "normal";
	}
	
	public boolean wall(int[] from, Direction to) {
		List<CellObject> walls = this.getCell(from).getAllWithProperty("block");
		if (walls != null) {
			for (CellObject wall : walls) {
				if (wall.getProperty("direction").equals(to.id())) {
					return true;
				}
			}
		}
		return false;
	}
}
