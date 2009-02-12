package org.msoar.gridmap2d;

public enum Direction {
	NONE (0, 0, Names.kNone, 0, new int[] {0, 0}) {
	},
	
	NORTH (1, 1, Names.kNorth, 3 * Math.PI / 2, new int[] {0, -1}) {
	}, 
	
	EAST (2, 2, Names.kEast, 0, new int[] {1, 0}) {
	},
	
	SOUTH (3, 4, Names.kSouth, Math.PI / 2, new int[] {0, 1}) {
	}, 
	
	WEST (4, 8, Names.kWest, Math.PI, new int[] {-1, 0}) {
	};
	
	private final int index;
	private final int indicator;
	private final String id;
	private final double radians;
	private final int[] delta;
	private Direction backward;
	private Direction left;
	private Direction right;
	
	static {
		Direction.NORTH.backward = Direction.SOUTH;
		Direction.EAST.backward = Direction.WEST;
		Direction.SOUTH.backward = Direction.NORTH;
		Direction.WEST.backward = Direction.EAST;

		Direction.NORTH.left = Direction.WEST;
		Direction.EAST.left = Direction.NORTH;
		Direction.SOUTH.left = Direction.EAST;
		Direction.WEST.left = Direction.SOUTH;

		Direction.NORTH.right = Direction.EAST;
		Direction.EAST.right = Direction.SOUTH;
		Direction.SOUTH.right = Direction.WEST;
		Direction.WEST.right = Direction.NORTH;
	}

	Direction(int index, int indicator, String id, double radians, int[] delta) {
		this.index = index;
		this.indicator = indicator;
		this.id = id;
		this.radians = radians;
		this.delta = delta;
	}
	
	public int index() { 
		return index; 
	}
	public int indicator() { 
		return indicator; 
	}
	public String id() { 
		return id; 
	}
	public double radians() { 
		return radians; 
	}
	public Direction backward() { 
		return backward; 
	}
	public Direction left() { 
		return left; 
	}
	public Direction right() { 
		return right; 
	}
	public static Direction parse(String name) {
		if (name.length() == 0) {
			return null;
		}
		
		switch (name.charAt(0)) {
		case 'N':
		case 'n':
			if (name.equalsIgnoreCase(Direction.NONE.id)) {
				return Direction.NONE;
			} 
			// assume north
			return Direction.NORTH;
		case 'E':
		case 'e':
			return Direction.EAST;
		case 'S':
		case 's':
			return Direction.SOUTH;
		case 'W':
		case 'w':
			return Direction.WEST;
		default:
			break;
		}
		return null;
	}
	
	// translate point toward cell in direction, storing result in destination
	// if destination null, use point
	public static int[] translate(int[] point, Direction direction, int[] destination) {
		if (destination == null) {
			destination = point;
		}
		destination[0] = point[0] + direction.delta[0];
		destination[1] = point[1] + direction.delta[1];
		return destination;
	}

	public static int[] translate(int[] point, Direction direction) {
		return translate(point, direction, null);
	}

	public static double toDisplayRadians(double internalRadians) {
		while (internalRadians < 0) {
			internalRadians += 2 * Math.PI;
		}
		return fmod(internalRadians + Math.PI / 2, 2 * Math.PI);
	}

	public static double toInternalRadians(double displayRadians) {
		while (displayRadians < 0) {
			displayRadians += 2 * Math.PI;
		}
		displayRadians = fmod(displayRadians - Math.PI / 2, 2 * Math.PI);
		while (displayRadians < 0) {
			displayRadians += 2 * Math.PI;
		}
		return displayRadians;
	}

	public static double fmod(double a, double mod) {
		double result = a;
		assert mod > 0;
		while (Math.abs(result) >= mod) {
			if (result > 0) {
				result -= mod;
			} else {
				result += mod;
			}
		}
		return result;
	}
}
