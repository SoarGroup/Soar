package org.msoar.gridmap2d.players;

import org.apache.log4j.Logger;
import org.msoar.gridmap2d.Gridmap2D;
import org.msoar.gridmap2d.Simulation;
import org.msoar.gridmap2d.map.TaxiMap;

public class Taxi extends Player {	
	private static Logger logger = Logger.getLogger(Taxi.class);

	private TaxiCommander commander;

	private int fuel;
	private int fuelStartMin;
	private int fuelStartMax;
	private int refuel;
	private boolean disableFuel;

	public Taxi(String playerId, int fuelStartMin, int fuelStartMax, int refuel, boolean disableFuel) throws Exception {
		super(playerId);
		
		this.fuelStartMin = fuelStartMin;
		this.fuelStartMax = fuelStartMax;
		this.refuel = refuel;
		this.disableFuel = disableFuel;
		
		reset();
	}
	
	public void setCommander(TaxiCommander commander) {
		this.commander = commander;
	}
	
	public CommandInfo getCommand() throws Exception {
		CommandInfo command;
		if (commander != null) {
			command = commander.nextCommand();
		} else {
			command = Gridmap2D.control.getHumanCommand(this);
		}
		
		return command;
	}
	
	public void update(int[] newLocation, TaxiMap taxiMap) throws Exception {
		super.update(newLocation);
		if (commander != null) {
			commander.update(taxiMap);
		}
	}
	
	@Override
	public void reset() throws Exception {
		super.reset();

		fuel = Simulation.random.nextInt(1 + fuelStartMax - fuelStartMin);
		fuel += fuelStartMin;
		
		if (commander != null) {
			commander.reset();
		}
	}

	public void shutdownCommander() throws Exception {
		if (commander != null) {
			commander.shutdown();
		}
	}
	
	public void consumeFuel() {
		if (disableFuel) {
			logger.info("fuel consuption disabled");
			return;
		}
		logger.info("fuel: " + Integer.toString(fuel) + " -> " + Integer.toString(fuel-1));
		fuel -= 1;
	}
	
	public int getFuel() {
		return fuel;
	}

	public void fillUp() {
		logger.info("fuel: " + Integer.toString(fuel) + " -> " + refuel + " (fillup)");
		fuel = refuel;
	}
}
