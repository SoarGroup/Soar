package eaters;

import java.util.HashMap;

public class CellType {
	public static final String kWall = "wall";
	public static final String kEmpty = "empty";
	
	public static final CellType WALL = new CellType(kWall);
	public static final CellType EMPTY = new CellType(kEmpty);
	
	private static HashMap cellTypesHash = new HashMap();
	
	static {
		cellTypesHash.put(kWall, WALL);
		cellTypesHash.put(kEmpty, EMPTY);
	}
	
	public static CellType getType(String name) {
		if (cellTypesHash.containsKey(name)) {
			return (CellType)cellTypesHash.get(name);
		}
		return null;
	}

	private final String name;
	private final int id;
	private static int nextID = 0;
	
	private CellType(String name) {
		this.name = name;
		this.id = nextID;
		++nextID;
	}
	
	public int id() {
		return id;
	}
	
	public String name() {
		return name;
	}
	
	public boolean equals(CellType type) {
		return this.id == type.id;
	}
}
