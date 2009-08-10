package edu.umich.soar.gridmap2d.map;

import java.util.Arrays;

import edu.umich.soar.config.Config;
import edu.umich.soar.config.ConfigFile;

public class CellObject {
	private final Config config;

	private boolean applyable;
	private boolean updatable;
	private int[] location;
	
	CellObject(CellObject other) {
		this.config = other.config.copy();
		this.applyable = other.applyable;
		this.updatable = other.updatable;
	}
	
	CellObject(Config config) {
		// create a new config in memory
		this.config = new Config(new ConfigFile());
		
		// copy over its stuff
		for (String key : config.getKeys()) {
			this.config.setStrings(key, config.getStrings(key));
		}
		
		updateCachedState();
	}
	
	public int[] getLocation() {
		if (location == null) {
			return null;
		}
		return Arrays.copyOf(location, location.length);
	}
	
	public void setLocation(int[] location) {
		if (location == null) {
			this.location = null;
			return;
		}
		this.location = Arrays.copyOf(location, location.length);
	}

	private void updateCachedState() {
		this.applyable = false;
		this.updatable = false;
		for (String key : config.getKeys()) {
			// set state
			this.applyable = this.applyable || key.startsWith("apply");
			this.updatable = this.updatable || key.startsWith("update");
		}
	}
	
	public boolean updatable() {
		return updatable;
	}
	
	public boolean applyable() {
		return applyable;
	}
	
	public void applyProperties() {
		// copy apply properties to regular properties
		Config applyProperties = this.config.getChild("apply.properties");
		for (String property : applyProperties.getKeys()) {
			this.config.setStrings(property, applyProperties.requireStrings(property));
			this.config.removeKey(property);
		}
		updateCachedState();
	}

	public String[] getPropertyList() {
		return config.getKeys();
	}
	
	public boolean hasProperty(String key) {
		return config.hasKey(key);
	}
	
	public void removeProperty(String key) {
		config.removeKey(key);
		updateCachedState();
	}
	
	// string
	public String getProperty(String key) {
		return config.getString(key);
	}

	public void setProperty(String key, String value) {
		config.setString(key, value);
		updateCachedState();
	}

	// string[]
	public String[] getPropertyArray(String key) {
		return config.getStrings(key);
	}

	public void setPropertyArray(String key, String[] values) {
		config.setStrings(key, values);
		updateCachedState();
	}
	
	// boolean
	public boolean getBooleanProperty(String key, boolean def) {
		return config.getBoolean(key, def);
	}

	public void setBooleanProperty(String key, boolean value) {
		config.setBoolean(key, value);
		updateCachedState();
	}
	
	// int
	public int getIntProperty(String key, int def) {
		return config.getInt(key, def);
	}

	public void setIntProperty(String key, int value) {
		config.setInt(key, value);
		updateCachedState();
	}
	
	// double
	public double getDoubleProperty(String key, double def) {
		return config.getDouble(key, def);
	}

	public void setDoubleProperty(String key, double value) {
		config.setDouble(key, value);
		updateCachedState();
	}
	
	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder();
		builder.append("(");
		for (String key : config.getKeys()) {
			builder.append(key);
			builder.append(" => ");
			builder.append(Arrays.toString(config.getStrings(key)));
			builder.append(", ");
		}
		builder.append(")");
		return builder.toString();
	}
}
