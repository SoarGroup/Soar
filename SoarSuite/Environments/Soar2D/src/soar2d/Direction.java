package soar2d;

import java.util.HashMap;

public class Direction {
	
	public final static int kNorthIndicator = 1;
	public final static int kEastIndicator = 2;
	public final static int kSouthIndicator = 4;
	public final static int kWestIndicator = 8;
	
	public final static int kNorthInt = 1;
	public final static int kEastInt = 2;
	public final static int kSouthInt = 3;
	public final static int kWestInt = 4;
	
	public final static Integer kNorthInteger = new Integer(kNorthInt);
	public final static Integer kEastInteger = new Integer(kEastInt);
	public final static Integer kSouthInteger = new Integer(kSouthInt);
	public final static Integer kWestInteger = new Integer(kWestInt);
	
	public final static String[] stringOf = new String[5];
	public final static int[] indicators = new int[5];
	public final static int[] backwardOf = new int[5];
	public final static int[] leftOf = new int[5];
	public final static int[] rightOf = new int[5];
	public final static int[] xDelta = new int[5];
	public final static int[] yDelta = new int[5];

	private final static HashMap<String, Integer> ints = new HashMap<String, Integer>(4);
	
	static {
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
	
	public static int getInt(String direction) {
		Integer d = ints.get(direction);
		return d.intValue();
	}
	
	public static void translate(java.awt.Point p, int direction) {
		assert direction > 0;
		assert direction < 5;
		
		p.x += xDelta[direction];
		p.y += yDelta[direction];
	}
	
	private Direction() {
	}
}

