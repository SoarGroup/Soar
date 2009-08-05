package edu.umich.soar.gridmap2d.players;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.Direction;
import edu.umich.soar.gridmap2d.Gridmap2D;


public class TankState {
	private static Logger logger = Logger.getLogger(TankState.class);

	public static final int RADAR_WIDTH = 3;
	public static final int RADAR_HEIGHT = 15;

	private String name;
	private int missiles;
	private int energy;
	private int health;
	private boolean shieldsUp;
	private boolean radarSwitch;
	private int radarPower;
	private int observedPower;
	private RadarCell[][] radar;
	private int rwaves;
	private int blocked;
	private int incoming;
	private int resurrectFrame = 0;
	private int smellDistance;
	private String smellColor;
	private Direction sound;
	private boolean onHealthCharger;
	private boolean onEnergyCharger;
	private int initialMissiles;
	private int initialEnergy;
	private int initialHealth;
	
	TankState(String name, Tank.Builder builder) {
		
		this.name = name;
		
		this.missiles = builder.missiles;
		this.initialMissiles = builder.missiles;
		
		this.energy = builder.energy;
		this.initialEnergy = builder.energy;
		
		this.health = builder.health;
		this.initialHealth = builder.health;
	}
	
	public void resetSensors() {
		// TODO: doing this after a getCommand seems like the wrong thing to do.
		rwaves = 0;
		incoming = 0;
		blocked = 0;
		smellDistance = 0;
		smellColor = null;
		sound = Direction.NONE;
		onHealthCharger = false;
		onEnergyCharger = false;
	}

	public int getEnergy() {
		return energy;
	}
	public void setEnergy(int energy) {
		setEnergy(energy, null);
	}
	public void setEnergy(int energy, String comment) {
		// Bring down shields if out of energy

		this.energy = energy;
		if (logger.isDebugEnabled()) {
			if (comment != null) {
				logger.debug(this.name + " energy set to: " + Integer.toString(this.energy) + " (" + comment + ")");
			} else {
				logger.debug(this.name + " energy set to: " + Integer.toString(this.energy));
			}
		}
	}
	public void adjustEnergy(int delta) {
		adjustEnergy(delta, null);
	}
	public void adjustEnergy(int delta, String comment) {
		// Bring down shields if out of energy

		int previous = this.energy;
		this.energy += delta;
		if (energy < 0) {
			energy = 0;
		}
		if (energy > Gridmap2D.config.tanksoarConfig().max_energy) {
			energy = Gridmap2D.config.tanksoarConfig().max_energy;
		}
		if (energy == previous) {
			return;
		}
		if (logger.isDebugEnabled()) {
			if (comment != null) {
				logger.debug(this.name + " energy: " + Integer.toString(previous) + " -> " + Integer.toString(this.energy) + " (" + comment + ")");
			} else {
				logger.debug(this.name + " energy: " + Integer.toString(previous) + " -> " + Integer.toString(this.energy));
			}
		}
	}
	public int getBlocked() {
		return blocked;
	}
	public void setBlocked(int blocked) {
		this.blocked = blocked;
	}
	public int getMissiles() {
		return missiles;
	}
	public void setMissiles(int missiles) {
		setMissiles(missiles, null);
	}
	public void setMissiles(int missiles, String comment) {
		this.missiles = missiles;
		if (logger.isDebugEnabled()) {
			if (comment != null) {
				logger.debug(this.name + " missiles set to: " + Integer.toString(this.missiles) + " (" + comment + ")");
			} else {
				logger.debug(this.name + " missiles set to: " + Integer.toString(this.missiles));
			}
		}
	}
	public void adjustMissiles(int delta) {
		adjustMissiles(delta, null);
	}
	public void adjustMissiles(int delta, String comment) {
		int previous = this.missiles;
		this.missiles += delta;
		if (missiles < 0) {
			logger.warn(this.name + ": missiles adjusted to negative value");
			missiles = 0;
		}
		if (missiles == previous) {
			return;
		}
		if (logger.isDebugEnabled()) {
			if (comment != null) {
				logger.debug(this.name + " missiles: " + Integer.toString(previous) + " -> " + Integer.toString(this.missiles) + " (" + comment + ")");
			} else {
				logger.debug(this.name + " missiles: " + Integer.toString(previous) + " -> " + Integer.toString(this.missiles));
			}
		}
	}
	public int getHealth() {
		return health;
	}
	public void setHealth(int health) {
		setHealth(health, null);
	}
	public void setHealth(int health, String comment) {
		this.health = health;
		if (logger.isDebugEnabled()) {
			if (comment != null) {
				logger.debug(this.name + " health set to: " + Integer.toString(this.health) + " (" + comment + ")");
			} else {
				logger.debug(this.name + " health set to: " + Integer.toString(this.health));
			}
		}
	}
	public void adjustHealth(int delta) {
		adjustHealth(delta, null);
	}
	public void adjustHealth(int delta, String comment) {
		int previous = this.health;
		this.health += delta;
		if (health < 0) {
			health = 0;
		}
		if (health > Gridmap2D.config.tanksoarConfig().max_health) {
			health = Gridmap2D.config.tanksoarConfig().max_health;
		}
		if (health == previous) {
			return;
		}
		if (logger.isDebugEnabled()) {
			if (comment != null) {
				logger.debug(this.name + " health: " + Integer.toString(previous) + " -> " + Integer.toString(this.health) + " (" + comment + ")");
			} else {
				logger.debug(this.name + " health: " + Integer.toString(previous) + " -> " + Integer.toString(this.health));
			}
		}
	}
	public boolean getShieldsUp() {
		return shieldsUp;
	}
	public void setShieldsUp(boolean shieldsUp) {
		if (this.shieldsUp == shieldsUp) {
			return;
		}
		if (logger.isDebugEnabled()) {
			logger.debug(this.name + " shields switched " + (shieldsUp ? "on" : "off"));
		}
		this.shieldsUp = shieldsUp;
	}
	public boolean getRadarSwitch() {
		return radarSwitch;
	}
	public void setRadarSwitch(boolean radarSwitch) {
		if (this.radarSwitch == radarSwitch ) {
			return;
		}
		this.radarSwitch = radarSwitch;
		if (logger.isDebugEnabled()) { 
			logger.debug(this.name + " radar switched " + (radarSwitch ? "on" : "off"));
		}
	}
	public int getRadarPower() {
		return radarPower;
	}
	public void setRadarPower(int radarPower) {
		if (radarPower < 0) {
			radarPower = 0;
		}
		if (radarPower >= RADAR_HEIGHT) {
			radarPower = RADAR_HEIGHT - 1;
		}
		if (this.radarPower == radarPower) {
			return;
		}
		this.radarPower = radarPower;
		if (logger.isDebugEnabled()) { 
			logger.debug(this.name + " radar power set to: " + Integer.toString(this.radarPower));
		}
	}
	public int getObservedPower() {
		return observedPower;
	}
	public void setObservedPower(int observedPower) {
		this.observedPower = observedPower;
	}
	public RadarCell[][] getRadar() {
		return radar;
	}
	public void setRadar(RadarCell[][] radar) {
		this.radar = radar;
	}
	public void clearRadar() {
		this.radar = new RadarCell[RADAR_WIDTH][RADAR_HEIGHT];
	}
	public int getRwaves() {
		return rwaves;
	}
	public void setRwaves(int rwaves) {
		this.rwaves = rwaves;
	}
	public void radarTouch(Direction fromDirection) {
		rwaves |= fromDirection.indicator();
	}
	public int getIncoming() {
		return incoming;
	}
	public void setIncoming(int incoming) {
		this.incoming = incoming;
	}
	public void setIncoming(Direction fromDirection) {
		incoming |= fromDirection.indicator();
	}
	public int getResurrectFrame() {
		return resurrectFrame;
	}
	public void setResurrectFrame(int resurrectFrame) {
		this.resurrectFrame = resurrectFrame;
	}
	public int getSmellDistance() {
		return smellDistance;
	}
	public void setSmellDistance(int smellDistance) {
		this.smellDistance = smellDistance;
	}
	public String getSmellColor() {
		return smellColor;
	}
	public void setSmellColor(String smellColor) {
		this.smellColor = smellColor;
	}
	public Direction getSound() {
		return sound;
	}
	public void setSound(Direction sound) {
		this.sound = sound;
	}
	public boolean getOnHealthCharger() {
		return onHealthCharger;
	}
	public void setOnHealthCharger(boolean onHealthCharger) {
		this.onHealthCharger = onHealthCharger;
	}
	public boolean getOnEnergyCharger() {
		return onEnergyCharger;
	}
	public void setOnEnergyCharger(boolean onEnergyCharger) {
		this.onEnergyCharger = onEnergyCharger;
	}

	public void reset() {
		if (this.initialMissiles > 0) {
			this.missiles = this.initialMissiles;
		} else {
			this.missiles = Gridmap2D.config.tanksoarConfig().max_missiles;
		}
		if (this.initialEnergy > 0) {
			this.health = this.initialEnergy;
		} else {
			this.health = Gridmap2D.config.tanksoarConfig().max_health;
		}
		if (this.initialHealth > 0) {
			this.energy = this.initialHealth;
		} else {
			this.energy = Gridmap2D.config.tanksoarConfig().max_energy;
		}
		
		this.shieldsUp = false;
		this.radarSwitch = false;
		this.radarPower = 0;
		this.resurrectFrame = Gridmap2D.simulation.getWorldCount();
		this.clearRadar();
		this.resetSensors();
	}

	public void fragged() {
		this.energy = Gridmap2D.config.tanksoarConfig().max_energy;
		this.health = Gridmap2D.config.tanksoarConfig().max_health;
		this.missiles = Gridmap2D.config.tanksoarConfig().max_missiles;
		this.resurrectFrame = Gridmap2D.simulation.getWorldCount(); 
		this.clearRadar();
		this.resetSensors();
	}
}

