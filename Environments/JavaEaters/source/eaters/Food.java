package eaters;

import java.util.HashMap;

public class Food {
	private static Food[] foodArray = new Food[0];
	private static HashMap foodHash = new HashMap();
	
	public static int foodTypeCount() {
		return foodArray.length;
	}
	
	/**
	* Reallocates an array with a new size, and copies the contents
	* of the old array to the new array.
	* @param oldArray  the old array, to be reallocated.
	* @param newSize   the new array size.
	* @return          A new array with the same contents.
	*/
	private static Object resizeArray(Object oldArray, int newSize) {
	   int oldSize = java.lang.reflect.Array.getLength(oldArray);
	   Class elementType = oldArray.getClass().getComponentType();
	   Object newArray = java.lang.reflect.Array.newInstance(elementType,newSize);
	   int preserveLength = Math.min(oldSize,newSize);
	   if (preserveLength > 0) {
	      System.arraycopy (oldArray,0,newArray,0,preserveLength);
	   }
	   return newArray; 
	}
	
	public static Food addFood(String name, Shape shape, String color, int value) {
		if (foodHash.containsKey(name)) {
			return null;
		}
		Food newFood = new Food(name, shape, color, value);
		foodArray = (Food[])resizeArray(foodArray, newFood.id + 1);
		foodArray[newFood.id] = newFood;
		foodHash.put(name, newFood);
		return newFood;
	}

	public static Food getFood(int id) {
		assert id >= 0;
		assert id < foodArray.length;
		return foodArray[id];
	}
	
	public static Food getFood(String name) {
		if (foodHash.containsKey(name)) {
			return (Food)foodHash.get(name);
		}
		return null;
	}
	
	private final String name;
	private final Shape shape;
	private final String color;
	private final int id;
	private int value;

	private Food(String name, Shape shape, String color, int value) {
		this.name = name;
		this.shape = shape;
		this.color = color;
		this.id = foodArray.length;
		this.value = value;
	}
	
	public String name() {
		return name;
	}
	
	public String color() {
		return color;
	}
	
	public Shape shape() {
		return shape;
	}
	
	public int id() {
		return id;
	}
	
	public int value() {
		return value;
	}
	
	public static void decay() {
		// This function is called if decay=true (in map file) and if so, after
		// each world update.
		// Example food decay function:
		// Remove one from the value of each type of food after each update.
		for (int i = 0; i < foodArray.length; ++i) {
			--foodArray[i].value;
		}
	}
}

