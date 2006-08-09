package eaters;

public class Food {
	public static final String kRound = "round";
	public static final String kSquare = "square";
		
	public static final int kRoundInt = 0;
	public static final int kSquareInt = 1;
		
	String m_Name;
	int m_Value;
	int m_Shape;
	String m_ColorString;
	static Food[] food;

	static int getFoodIndexByName(String name) {
		for (int i = 0; i < food.length; ++i) {
			if (food[i].getName().equalsIgnoreCase(name)) {
				return i;
			}
		}
		return -1;
	}

	public Food(String name, int value, String shape, String color) {
		m_Name = name;
		m_Value = value;
		if (shape.equalsIgnoreCase(kRound)) {
			m_Shape = kRoundInt;
		} else if (shape.equalsIgnoreCase(kSquare)) {
			m_Shape = kSquareInt;
		}
		m_ColorString = color;
	}
	
	public String getColor() {
		return m_ColorString;
	}
	
	public String getName() {
		return m_Name;
	}
	
	public int getValue() {
		return m_Value;
	}
	
	public int getShape() {
		return m_Shape;
	}
	
	public String getShapeName() {
		switch (m_Shape) {
		case kRoundInt:
			return kRound;
		case kSquareInt:
			return kSquare;
		}
		return null;
	}
}

