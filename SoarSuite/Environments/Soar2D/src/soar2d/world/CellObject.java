package soar2d.world;

import java.util.*;

import soar2d.*;
import soar2d.player.Player;

public class CellObject {
	HashMap<String, String> properties = new HashMap<String, String>();
	HashMap<String, String> propertiesApply = new HashMap<String, String>();
	String name;
	boolean updatable;
	boolean consumable;

	boolean pointsApply = false;
	boolean decayUpdate = false;
	
	public CellObject(CellObject cellObject) {
		this.properties = new HashMap<String, String>(cellObject.properties);
		this.name = new String(cellObject.name);
		this.updatable = cellObject.updatable;
		this.consumable = cellObject.consumable;
		this.pointsApply = cellObject.pointsApply;
		this.decayUpdate = cellObject.decayUpdate;
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
	
	public boolean addPropertyApply(String name, String value) {
		if (propertiesApply.containsKey(name)) {
			return false;
		}
		propertiesApply.put(name, value);
		return true;
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
	
	public void setDecayUpdate(boolean setting) {
		decayUpdate = setting;
	}
	
	public boolean apply(Player player) {
		if (pointsApply) {
			assert properties.containsKey(Names.kPropertyPoints);
			int points = Integer.parseInt(properties.get(Names.kPropertyPoints));
			player.adjustPoints(points, name);
		}
		
		if (propertiesApply.size() > 0) {
			Iterator<String> iter = propertiesApply.keySet().iterator();
			while (iter.hasNext()) {
				String key = iter.next();
				String value = propertiesApply.get(key);
				Soar2D.logger.info("Box opened, new property: " + key + " --> " + value);
				properties.put(key, value);
			}
		}
		return consumable;	// if this is true the object is removed from 
							// the cell after the apply
	}
	public boolean update(World world, java.awt.Point location) {
		if (decayUpdate) {
			assert properties.containsKey(Names.kPropertyPoints);
			int points = Integer.parseInt(properties.get(Names.kPropertyPoints));
			points -= 1;
			properties.put(Names.kPropertyPoints, Integer.toString(points));
			if (points == 0) {
				return true;	// this causes this object to be removed from the cell
			}
		}
		return false; // this keeps this object around
	}
	
	public boolean hasProperty(String name) {
		if (properties.containsKey(name)) {
			return true;
		}
		return false;
	}
	
	public Set<String> getPropertyNames() {
		return properties.keySet();
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
