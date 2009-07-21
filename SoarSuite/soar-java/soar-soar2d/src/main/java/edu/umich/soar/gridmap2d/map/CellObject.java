package edu.umich.soar.gridmap2d.map;

import java.util.Arrays;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.config.Config;
import edu.umich.soar.gridmap2d.config.ConfigFile;


// TODO: use string constants

public class CellObject {
	private static Logger logger = Logger.getLogger(CellObject.class);
	private static long idCount = 0;

	private Config config;
	private Long id;
	private boolean applyable;
	private boolean updatable;
	
	CellObject(CellObject cellObject) {
		this.id = new Long(idCount++);
		this.config = cellObject.config.copy();
		this.applyable = cellObject.applyable;
		this.updatable = cellObject.updatable;
	}
	
	CellObject(Config config) {
		logger.trace("Creating object: " + config.requireString("name"));
		
		this.id = new Long(idCount++);

		// create a new config in memory
		this.config = new Config(new ConfigFile());
		
		// copy over its stuff
		for (String key : config.getKeys()) {
			this.config.setStrings(key, config.getStrings(key));
		}
		
		updateCachedState();
	}
	
	public boolean equals(CellObject other) {
		return id == other.id;
	}
	
	public String getName() {
		return config.requireString("name");
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
			// TODO: make sure this works!
			this.config.removeKey(property);
		}
		updateCachedState();
	}

	public String[] getPropertyList() {
		return config.getKeys();
	}
	
	public boolean hasProperty(String name) {
		return config.hasKey(name);
	}
	
	public void removeProperty(String property) {
		config.removeKey(property);
		updateCachedState();
	}
	
	// string
	public String getProperty(String name) {
		return config.getString(name);
	}

	public void setProperty(String name, String value) {
		config.setString(name, value);
		updateCachedState();
	}

	// string[]
	public String[] getPropertyArray(String name) {
		return config.getStrings(name);
	}

	public void setPropertyArray(String name, String[] values) {
		config.setStrings(name, values);
		updateCachedState();
	}
	
	// boolean
	public boolean getBooleanProperty(String name, boolean def) {
		return config.getBoolean(name, def);
	}

	public void setBooleanProperty(String name, boolean value) {
		config.setBoolean(name, value);
		updateCachedState();
	}
	
	// int
	public int getIntProperty(String name, int def) {
		return config.getInt(name, def);
	}

	public void setIntProperty(String name, int value) {
		config.setInt(name, value);
		updateCachedState();
	}
	
	// double
	public double getDoubleProperty(String name, double def) {
		return config.getDouble(name, def);
	}

	public void setDoubleProperty(String name, double value) {
		config.setDouble(name, value);
		updateCachedState();
	}
	
	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder();
		builder.append(config.getString("name"));
		builder.append(": ");
		for (String key : config.getKeys()) {
			if (key.equals("name")) {
				continue;
			}
			builder.append(key);
			builder.append(" => ");
			builder.append(Arrays.toString(config.getStrings(key)));
			builder.append(", ");
		}
		return builder.toString();
	}
}
