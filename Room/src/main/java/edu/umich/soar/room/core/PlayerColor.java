package edu.umich.soar.room.core;

import java.util.ArrayList;
import java.util.List;

public enum PlayerColor {
	RED, 
	BLUE, 
	GREEN, 
	PURPLE, 
	ORANGE, 
	BLACK, 
	YELLOW;
	
	private static final List<PlayerColor> unusedColors = new ArrayList<PlayerColor>(PlayerColor.values().length);

	static {
		for (PlayerColor color : PlayerColor.values()) {
			unusedColors.add(color);
		}
	}
	
	public static List<PlayerColor> getUnusedColors() {
		return new ArrayList<PlayerColor>(unusedColors);
	}
	
	public boolean use() {
		synchronized(unusedColors) {
			return unusedColors.remove(this);
		}
	}
	
	public void free() {
		synchronized(unusedColors) {
			if (!unusedColors.contains(this)) {
				unusedColors.add(this);
			}
		}
	}
	
	public static PlayerColor useNext() {
		synchronized(unusedColors) {
			if (unusedColors.isEmpty()) {
				return null;
			}
			return unusedColors.remove(0);
		}		
	}
	
	public boolean isUsed() {
		synchronized(unusedColors) {
			return !unusedColors.contains(this);
		}		
	}
}

