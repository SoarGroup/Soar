package soar2d.world;

import java.util.*;

import soar2d.*;

public class CellObject {
	// Factory methods and members
	private static HashMap<String, CellObject> templates = new HashMap<String, CellObject>();
	public static boolean hasTemplate(String name) {
		return templates.containsKey(name);
	}
	public static boolean removeTemplate(String name, CellObject cellObject) {
		if (templates.containsKey(name)) {
			templates.remove(name);
			return true;
		}
		return false;
	}
	public static boolean registerTemplate(String name, CellObject cellObject) {
		if (templates.containsKey(name)) {
			return false;
		}
		if (cellObject == null) {
			return false;
		}
		templates.put(name, cellObject);
		return true;
	}
	public static CellObject createObject(String name) {
		if (templates.containsKey(name)) {
			return new CellObject(templates.get(name));
		}
		return null;
	}
	// End factory methods and members
	
	HashMap<String, Object> properties = new HashMap<String, Object>();
	String name;
	boolean updatable;
	boolean consumable;

	boolean pointsApply = false;
	
	public CellObject(CellObject cellObject) {
		this.properties = new HashMap<String, Object>(cellObject.properties);
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
	
	public boolean addProperty(String name, Object value) {
		if (properties.containsKey(name)) {
			return false;
		}
		properties.put(name, value);
		return true;
	}
	
	public void setPointsApply(boolean setting) {
		pointsApply = setting;
	}
	
	public boolean apply(Entity entity) {
		if (pointsApply) {
			entity.adjustPoints(((Integer)properties.get(Names.kPropertyPoints)).intValue(), name);
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
