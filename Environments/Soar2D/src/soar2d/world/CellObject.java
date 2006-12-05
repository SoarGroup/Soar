package soar2d.world;

import java.util.*;

import soar2d.*;
import soar2d.player.Player;

public class CellObject {
	HashMap<String, String> properties = new HashMap<String, String>();
	String name;
	boolean updatable;
	boolean consumable;

	boolean pointsApply = false;
	
	public CellObject(CellObject cellObject) {
		this.properties = new HashMap<String, String>(cellObject.properties);
		this.name = new String(cellObject.name);
		this.updatable = cellObject.updatable;
		this.consumable = cellObject.consumable;
		this.pointsApply = cellObject.pointsApply;
	}
	
	public CellObject(String name, boolean updatable, boolean consumable) {
		this.name = name;
		this.updatable = updatable;
		this.consumable = consumable;
	}
	
	public String getName() {
		return name;
	}
	public boolean updatable() {
		return updatable;
	}
	
	public boolean addProperty(String name, String value) {
		if (properties.containsKey(name)) {
			return false;
		}
		properties.put(name, value);
		return true;
	}
	
	public void setPointsApply(boolean setting) {
		pointsApply = setting;
	}
	
	public boolean apply(Player player) {
		if (pointsApply) {
			assert properties.containsKey(Names.kPropertyPoints);
			int points = Integer.parseInt(properties.get(Names.kPropertyPoints));
			player.adjustPoints(points, name);
		}
		return consumable;
	}
	public void update(World world, java.awt.Point location) {
	}
	
	public boolean hasProperty(String name) {
		if (properties.containsKey(name)) {
			return true;
		}
		return false;
	}
	
	public Object getProperty(String name) {
		if (properties.containsKey(name)) {
			return properties.get(name);
		}
		return null;
	}
	public String getStringProperty(String name) {
		if (properties.containsKey(name)) {
			return (String)properties.get(name);
		}
		return new String("");
	}
	public boolean getBooleanProperty(String name) {
		if (properties.containsKey(name)) {
			return Boolean.parseBoolean((String)properties.get(name));
		}
		return Soar2D.config.kDefaultPropertyBoolean;
	}
	public int getIntProperty(String name) {
		if (properties.containsKey(name)) {
			return Integer.parseInt((String)properties.get(name));
		}
		return Soar2D.config.kDefaultPropertyInt;
	}
	public float getFloatProperty(String name) {
		if (properties.containsKey(name)) {
			return Float.parseFloat((String)properties.get(name));
		}
		return Soar2D.config.kDefaultPropertyFloat;
	}
}
