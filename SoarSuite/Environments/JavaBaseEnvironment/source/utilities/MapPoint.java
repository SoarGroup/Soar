package utilities;

import java.util.logging.*;

import simulation.*;

public class MapPoint {
	private static Logger logger = Logger.getLogger("utilities");
	public int x;
	public int y;

	public MapPoint() {
		x = -1;
		y = -1;
	}
	
	public MapPoint(int x, int y) {
		this.x = x;
		this.y = y;
	}
	
	public MapPoint(MapPoint p) {
		this.x = p.x;
		this.y = p.y;
	}
	
	public String toString() {
		return "(" + x + "," + y + ")";
	}
	
	public MapPoint travel(int direction) {
		return new MapPoint(x + getXIncrement(direction), y + getYIncrement(direction));
	}
	
	public boolean equals(MapPoint p) {
		return (x == p.x) && (y == p.y);
	}
	
	public int directionTo(MapPoint p) {
		int direction = 0;
		if (p.x < x) {
			direction |= WorldEntity.kWestInt;
		} else if (p.x > x) {
			direction |= WorldEntity.kEastInt;
		}
		if (p.y < y) {
			direction |= WorldEntity.kNorthInt;
		} else if (p.y > y) {
			direction |= WorldEntity.kSouthInt;
		}
		return direction;
	}
	
	private static int getXIncrement(int direction) {
		switch (direction) {
		case WorldEntity.kWestInt:
			return -1;
		case WorldEntity.kEastInt:
			return 1;
		default:
			break;
		}		
		return 0;
	}
	
	private static int getYIncrement(int direction) {
		switch (direction) {
		case WorldEntity.kNorthInt:
			return -1;
		case WorldEntity.kSouthInt:
			return 1;
		default:
			break;
		}		
		return 0;
	}
	
	public int getManhattanDistanceTo(MapPoint point) {		
		return Math.abs(this.x - point.x) + Math.abs(this.y - point.y);
	}
}
