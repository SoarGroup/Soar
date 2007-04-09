package soar2d.world;

import java.util.*;

import soar2d.*;
import soar2d.player.Player;

/**
 * @author voigtjr
 *
 * This class represents contents of cells. Other special meta-objects are 
 * also used, like a redraw object to trigger the redrawing of a cell, or
 * an explosion object to trigger the drawing of an explosion.
 */
public class CellObject {
	private static final boolean kDefaultPropertyBoolean = false;	// if a bool property doesn't exist
	private static final float kDefaultPropertyFloat = 0;			// if a float property doesn't exist
	private static final int kDefaultPropertyInt = 0;				// if an int property doesn't exist

	/**
	 * Property list, name to value, both strings for simplicity
	 */
	HashMap<String, String> properties = new HashMap<String, String>();
	/**
	 * List of properties to apply to the object if this object is applied.
	 * Used for YJs boxes.
	 */
	HashMap<String, String> propertiesApply = new HashMap<String, String>();
	/**
	 * The name of this cell object.
	 */
	String name;

	/**
	 * remove the object after the apply
	 */
	boolean removeApply = false;
	/**
	 * do reward logic during the apply, the amount is the reward
	 * one positive reward per reward set, the rest are negative
	 * info box contains box-id of positive reward
	 */
	int rewardApply = 0;
	/**
	 * do reward information logic during the apply
	 */
	boolean rewardInfoApply = false;
	/**
	 * require a correct open code (open parameter) when opening boxes
	 * depends on rewardInfoApply
	 */
	boolean useOpenCode = false;
	/**
	 * perform a reset after this update, just like a terminal
	 */
	boolean resetApply = false;
	/**
	 * add the points property to the player during the apply.
	 * Points can be negative or zero.
	 */
	boolean pointsApply = false;
	/**
	 * add the missiles to the player during the apply.
	 * Missiles can be negative or zero.
	 */
	boolean missilesApply = false;
	/**
	 * add the health to the player during the apply.
	 * Health can be negative or zero.
	 */
	boolean healthApply = false;
	/**
	 * If this is true, only apply health if shields are down.
	 * Useful for attacking missiles.
	 */
	boolean healthApplyShieldsDown = false;
	/**
	 * add the energy to the player during the apply.
	 * energy can be negative or zero.
	 */
	boolean energyApply = false;
	/**
	 * If this is true, only apply energy if shields are up.
	 * Useful for attacking missiles.
	 */
	boolean energyApplyShieldsUp = false;
	/**
	 * Apply the decay property to this cell's points on an update.
	 */
	boolean decayUpdate = false;
	/**
	 * Apply the fly missile code on an update.
	 */
	boolean flyMissileUpdate = false;
	boolean lingerUpdate = false;
	
	private static int idCount = 0;
	private Integer id;
	
	CellObject(CellObject cellObject) {
		this.id = new Integer(idCount++);
		this.properties = new HashMap<String, String>(cellObject.properties);
		this.propertiesApply = new HashMap<String, String>(cellObject.propertiesApply);
		this.name = new String(cellObject.name);
		this.removeApply = cellObject.removeApply;
		this.rewardApply = cellObject.rewardApply;
		this.rewardInfoApply = cellObject.rewardInfoApply;
		this.useOpenCode = cellObject.useOpenCode;
		this.resetApply = cellObject.resetApply;
		this.pointsApply = cellObject.pointsApply;
		this.missilesApply = cellObject.missilesApply;
		this.healthApply = cellObject.healthApply;
		this.healthApplyShieldsDown = cellObject.healthApplyShieldsDown;
		this.energyApply = cellObject.energyApply;
		this.energyApplyShieldsUp = cellObject.energyApplyShieldsUp;
		this.decayUpdate = cellObject.decayUpdate;
		this.flyMissileUpdate = cellObject.flyMissileUpdate;
		this.lingerUpdate = cellObject.lingerUpdate;
	}
	
	CellObject(String name) {
		this.id = new Integer(idCount++);
		this.name = name;
	}
	
	public Integer getId() {
		return id;
	}
	
	public boolean equals(CellObject other) {
		return id == other.id;
	}
	
	public String getName() {
		return name;
	}
	public boolean updatable() {
		return this.decayUpdate
			|| this.flyMissileUpdate
			|| this.lingerUpdate;
	}
	
	public boolean applyable() {
		return (propertiesApply.size() > 0) 
			|| this.pointsApply 
			|| this.energyApply 
			|| this.healthApply 
			|| this.missilesApply 
			|| this.removeApply
			|| (this.rewardApply > 0)
			|| this.rewardInfoApply
			|| this.resetApply;
	}
	
	/**
	 * @param name property name
	 * @param value property value
	 * 
	 * Add a property apply, that is, a property that is added to this object
	 * when it is applied.
	 * Note, this overwrites an existing property if necessary.
	 */
	public void addPropertyApply(String name, String value) {
		propertiesApply.put(name, value);
	}
	public String getPropertyApply(String name) {
		return propertiesApply.get(name);
	}
	
	public boolean getResetApply() {
		return this.resetApply;
	}
	
	/**
	 * @param name property name
	 * @param value property value
	 * 
	 * Add a property to this object.
	 * Note, this overwrites an existing property if necessary.
	 */
	public void addProperty(String name, String value) {
		properties.put(name, value);
	}
	
	public void setRemoveApply(boolean setting) {
		removeApply = setting;
	}
	
	public void setRewardApply(int setting) {
		assert setting >= 0;
		rewardApply = setting;
	}
	
	public void setRewardInfoApply(boolean setting) {
		rewardInfoApply = setting;
	}
	
	public void setUseOpenCode(boolean setting) {
		useOpenCode = setting;
	}
	
	public void setResetApply(boolean setting) {
		resetApply = setting;
	}
	
	public void setPointsApply(boolean setting) {
		pointsApply = setting;
	}
	
	public void setMissilesApply(boolean setting) {
		missilesApply = setting;
	}
	
	public void setLingerUpdate(boolean setting) {
		lingerUpdate = setting;
	}
	
	/**
	 * @see energyApply
	 * @see energyApplyShieldsUp
	 */
	public void setEnergyApply(boolean setting, boolean onShieldsUp) {
		this.energyApply = setting;
		this.energyApplyShieldsUp = onShieldsUp;
	}
	
	/**
	 * @see healthApply
	 * @see healthApplyShieldsDown
	 */
	public void setHealthApply(boolean setting, boolean onShieldsDown) {
		this.healthApply = setting;
		this.healthApplyShieldsDown = onShieldsDown;
	}
	
	public void setDecayUpdate(boolean setting) {
		decayUpdate = setting;
	}
	
	public void setName(String name) {
		this.name = name;
	}
	
	public void setFlyMissileUpdate(boolean setting) {
		flyMissileUpdate = setting;
	}
	
	/**
	 * @param player called when this player acted on this object for whatever reason
	 * @return true if the object should be removed from the cell after the apply
	 */
	public boolean apply(World world, Player player) {
		if (propertiesApply.size() > 0) {
			Iterator<String> iter = propertiesApply.keySet().iterator();
			while (iter.hasNext()) {
				String key = iter.next();
				String value = propertiesApply.get(key);
				Soar2D.logger.info("New property: " + key + " --> " + value);
				properties.put(key, value);
			}
		}
		
		if (pointsApply) {
			assert properties.containsKey(Names.kPropertyPoints);
			int points = Integer.parseInt(properties.get(Names.kPropertyPoints));
			player.adjustPoints(points, name);
		}
		
		if (missilesApply) {
			assert properties.containsKey(Names.kPropertyMissiles);
			int missiles = Integer.parseInt(properties.get(Names.kPropertyMissiles));
			player.adjustMissiles(missiles, name);
		}
		
		if (energyApply) {
			if (!energyApplyShieldsUp || player.shieldsUp()) {
				assert properties.containsKey(Names.kPropertyEnergy);
				int energy = Integer.parseInt(properties.get(Names.kPropertyEnergy));
				player.adjustEnergy(energy, name);
			}
		}
		
		if (healthApply) {
			if (!healthApplyShieldsDown || !player.shieldsUp()) {
				assert properties.containsKey(Names.kPropertyHealth);
				int health = Integer.parseInt(properties.get(Names.kPropertyHealth));
				player.adjustHealth(health, name);
			}
		}
		
		if (rewardApply > 0) {
			assert properties.containsKey(Names.kPropertyBoxID);
			int myID = Integer.parseInt(properties.get(Names.kPropertyBoxID));
			
			// if we're not using an open code, the correct code is 0
			int usedOpenCode = 0;
			if (properties.containsKey(Names.kPropertyOpenCode)) {
				usedOpenCode = Integer.parseInt(properties.get(Names.kPropertyOpenCode));
			}

			// Set the reset if I'm the positive box
			this.resetApply = (myID == world.getMap().positiveRewardID);
			
			
			// if I am the positive box (resetApply == true, above) and we used the correct open code
			if (this.resetApply && (usedOpenCode == world.getMap().openCode)) {
				// reward positively
				player.adjustPoints(rewardApply, "positive reward");
			} else {
				// reward negatively
				player.adjustPoints(rewardApply * -1, "negative reward");
			}
		}
		
		if (removeApply) {
			return true;
		}
		
		return false;
	}
	/**
	 * @param world the world 
	 * @param location where this object is at
	 * @return true if the object should be removed from the cell after the update
	 * 
	 * This is called if the object is "updatable"
	 */
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
		
		if (flyMissileUpdate) {
			int phase = this.getIntProperty(Names.kPropertyFlyPhase);
			phase += 1;
			phase %= 4;
			this.addProperty(Names.kPropertyFlyPhase, Integer.toString(phase));
			return true;
		}
		
		if (lingerUpdate) {
			assert properties.containsKey(Names.kPropertyLinger);
			int remaining = Integer.parseInt(properties.get(Names.kPropertyLinger));
			remaining -= 1;
			properties.put(Names.kPropertyLinger, Integer.toString(remaining));
			if (remaining == 0) {
				return true;
			}
		}
		
		return false;
	}
	
	/**
	 * @param name the property to check for
	 * @return true if that property exists
	 * 
	 * Note: property foo -> false returns true here, this doesn't return the value
	 * of the property, but rather its existence.
	 */
	public boolean hasProperty(String name) {
		if (properties.containsKey(name)) {
			return true;
		}
		return false;
	}
	
	/**
	 * @return a set of property names
	 */
	public Set<String> getPropertyNames() {
		return properties.keySet();
	}
	/**
	 * @param name the property name
	 * @return the property value or null if it doesn't exist
	 */
	public String getProperty(String name) {
		if (properties.containsKey(name)) {
			return (String)properties.get(name);
		}
		return null;
	}
	/**
	 * @param name the property
	 * @return the string converted to a boolean
	 * 
	 * same as getProperty but converts it to boolean first
	 * use with care
	 */
	public boolean getBooleanProperty(String name) {
		if (properties.containsKey(name)) {
			return Boolean.parseBoolean((String)properties.get(name));
		}
		return kDefaultPropertyBoolean;
	}
	/**
	 * @param name the property
	 * @return the string converted to a int
	 * 
	 * same as getProperty but converts it to int first
	 * use with care
	 */
	public int getIntProperty(String name) {
		if (properties.containsKey(name)) {
			return Integer.parseInt((String)properties.get(name));
		}
		return kDefaultPropertyInt;
	}
	/**
	 * @param name the property
	 * @return the string converted to a float
	 * 
	 * same as getProperty but converts it to float first
	 * use with care
	 */
	public float getFloatProperty(String name) {
		if (properties.containsKey(name)) {
			return Float.parseFloat((String)properties.get(name));
		}
		return kDefaultPropertyFloat;
	}
}
