package soar2d;

import java.util.HashMap;

/**
 * @author voigtjr
 *
 * Direction class does a few funky things with numbers. The key thing to note:
 * 1 = north = kNorthInt
 * 2 = east = kEastInt
 * 3 = south = kSouthInt
 * 4 = west = kWestInt
 * 
 * Use stringOf to convert from "north" to 1
 * 
 * Use getInt to convert from 1 to "north"
 */
public class Direction {
	
	/**
	 * represents north
	 */
	public final static int kNorthInt = 1;
	/**
	 * represents east
	 */
	public final static int kEastInt = 2;
	/**
	 * represents south
	 */
	public final static int kSouthInt = 3;
	/**
	 * represents west
	 */
	public final static int kWestInt = 4;
	
	/**
	 * Used in tanksoar sensors
	 */
	public final static int kNorthIndicator = 1;
	/**
	 * Used in tanksoar sensors
	 */
	public final static int kEastIndicator = 2;
	/**
	 * Used in tanksoar sensors
	 */
	public final static int kSouthIndicator = 4;
	/**
	 * Used in tanksoar sensors
	 */
	public final static int kWestIndicator = 8;
	
	public final static Integer kNorthInteger = new Integer(kNorthInt);
	public final static Integer kEastInteger = new Integer(kEastInt);
	public final static Integer kSouthInteger = new Integer(kSouthInt);
	public final static Integer kWestInteger = new Integer(kWestInt);
	
	/**
	 * Use to convert an int to a string direction
	 */
	public final static String[] stringOf = new String[5];
	
	/**
	 * Use to convert from direction int to indicator int
	 */
	public final static int[] indicators = new int[5];
	/**
	 * Use to get direction opposite of current
	 */
	public final static int[] backwardOf = new int[5];
	/**
	 * Use to get direction left of current
	 */
	public final static int[] leftOf = new int[5];
	/**
	 * Use to get direction right of current
	 */
	public final static int[] rightOf = new int[5];
	/**
	 * Use to get change in x when travelling a certain direction
	 */
	public final static int[] xDelta = new int[5];
	/**
	 * Use to get change in y when travelling a certain direction
	 */
	public final static int[] yDelta = new int[5];

	/**
	 * used to get Integers froms strings
	 */
	private final static HashMap<String, Integer> ints = new HashMap<String, Integer>(4);
	
	static {
		// set all those data structures up
		
		ints.put(Names.kNorth, kNorthInteger);	// string -> int
		ints.put(Names.kEast, kEastInteger);
		ints.put(Names.kSouth, kSouthInteger);
		ints.put(Names.kWest, kWestInteger);
		
		indicators[0] = 0;
		indicators[kNorthInt] = kNorthIndicator;
		indicators[kEastInt] = kEastIndicator;
		indicators[kSouthInt] = kSouthIndicator;
		indicators[kWestInt] = kWestIndicator;
		
		stringOf[0] = null;					// int -> string
		stringOf[kNorthInt] = Names.kNorth;
		stringOf[kEastInt] = Names.kEast;
		stringOf[kSouthInt] = Names.kSouth;
		stringOf[kWestInt] = Names.kWest;
		
		backwardOf[0] = 0;					
		backwardOf[kNorthInt] = kSouthInt;	// backward of north is south
		backwardOf[kEastInt] = kWestInt;
		backwardOf[kSouthInt] = kNorthInt;
		backwardOf[kWestInt] = kEastInt;
		
		leftOf[0] = 0;				
		leftOf[kNorthInt] = kWestInt;		// left of north is west
		leftOf[kEastInt] = kNorthInt;
		leftOf[kSouthInt] = kEastInt;
		leftOf[kWestInt] = kSouthInt;
		
		rightOf[0] = 0;					
		rightOf[kNorthInt] = kEastInt;	// right of north is east
		rightOf[kEastInt] = kSouthInt;
		rightOf[kSouthInt] = kWestInt;
		rightOf[kWestInt] = kNorthInt;
		
		xDelta[0] = 0;					
		xDelta[kNorthInt] = 0;	
		xDelta[kEastInt] = 1;
		xDelta[kSouthInt] = 0;
		xDelta[kWestInt] = -1;
		
		yDelta[0] = 0;					
		yDelta[kNorthInt] = -1;	
		yDelta[kEastInt] = 0;
		yDelta[kSouthInt] = 1;
		yDelta[kWestInt] = 0;
	}
	
	/**
	 * @param direction the string direction to convert
	 * @return the integer direction, such as kNorthInt (the default when string invalid)
	 */
	public static int getInt(String direction) {
		Integer d = ints.get(direction);
		if (d == null) {
			return 0;
		}
		return d.intValue();
	}
	
	/**
	 * @param p the point to move
	 * @param direction the direction to move the point
	 * 
	 * takes a point p and translates it one unit in the direction indicated
	 */
	public static void translate(java.awt.Point p, int direction) {
		assert direction > 0;
		assert direction < 5;
		
		p.x += xDelta[direction];
		p.y += yDelta[direction];
	}
	
	private Direction() {
	}
}

