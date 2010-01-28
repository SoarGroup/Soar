package edu.umich.soar.room;

import java.awt.Color;

import edu.umich.soar.room.core.PlayerColor;

public class Colors {
	public static Color getColor(PlayerColor color) {
		return getColor(color.toString());
	}
	
	public static Color getColor(String color) {
		if (color == null) {
			return null;
		}
		if (color.equalsIgnoreCase("white")) {
			return Color.WHITE;
		}
		if (color.equalsIgnoreCase("blue")) {
			return Color.BLUE;
		}
		if (color.equalsIgnoreCase("red")) {
			return Color.RED;
		}
		if (color.equalsIgnoreCase("yellow")) {
			return Color.YELLOW;
		}
		if (color.equalsIgnoreCase("green")) {
			return Color.GREEN;
		}
		if (color.equalsIgnoreCase("purple")) {
			return Color.decode("0xA020F0");
		}
		if (color.equalsIgnoreCase("orange")) {
			return Color.ORANGE;
		}
		if (color.equalsIgnoreCase("black")) {
			return Color.BLACK;
		}
		if (color.equalsIgnoreCase("brown")) {
			return Color.decode("0xA52A2A");
		}
		if (color.equalsIgnoreCase("lightGray")) {
			return Color.LIGHT_GRAY;
		}
		if (color.equalsIgnoreCase("darkGray")) {
			return Color.DARK_GRAY;
		}
		return null;
	}


}
