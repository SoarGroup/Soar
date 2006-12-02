package soar2d.visuals;

import java.util.HashMap;

public class Shape {
	public static final String kRound = "round";
	public static final String kSquare = "square";
	
	public static final Shape ROUND = new Shape(kRound);
	public static final Shape SQUARE = new Shape(kSquare);
	
	private static HashMap<String, Shape> shapeHash = new HashMap<String, Shape>();
	
	static {
		shapeHash.put(kRound, ROUND);
		shapeHash.put(kSquare, SQUARE);
	}
	
	private static int nextID = 0;
	private final String name;
	private final int id;
	
	public static Shape getShape(String name) {
		return shapeHash.get(name);
	}
	
	private Shape(String name) {
		this.name = name;
		this.id = nextID;
		++nextID;
	}
	
	public int id() {
		return id;
	}
	
	public String toString() {
		return name;
	}
	
	public boolean equals(Shape shape) {
		return this.id == shape.id;
	}
}
