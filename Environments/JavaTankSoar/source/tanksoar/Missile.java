package tanksoar;

import java.util.logging.*;

import utilities.*;

public class Missile {
	private static Logger logger = Logger.getLogger("simulation");
	
	private MapPoint location;
	private int direction;
	private Tank owner;

	private static int nextID = 0;
	private int id;
	private int flightPhase; // 0, 1 == affects current location, 2 == affects current location + 1
	
	public Missile(MapPoint location, int direction, Tank owner) {
		this.location = location;
		this.direction = direction;
		this.owner = owner;

		id = nextID++;
		flightPhase = 0;
	}
	
	public Tank getOwner() {
		return owner;
	}
	
	public int getID() {
		return id;
	}
	
	void move() {
		if (flightPhase == 2) {
			location = location.travel(direction);
		}
		location = location.travel(direction);
		
		++flightPhase;
		flightPhase %= 3;
	}
	
	public MapPoint getLocation() {
		return location;
	}
	
	MapPoint[] getThreatenedLocations() {
		MapPoint[] threats;
		if (flightPhase == 2) {
			threats = new MapPoint[2];
			threats[0] = location;
			threats[1] = location.travel(direction);
		} else {
			threats = new MapPoint[1];
			threats[0] = location;
		}
		return threats;
	}
	
	public int getDirection() {
		return direction;
	}
	
	public int getFlightPhase() {
		return flightPhase;
	}
}

