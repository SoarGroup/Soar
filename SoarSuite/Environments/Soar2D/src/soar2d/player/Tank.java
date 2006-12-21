package soar2d.player;

import soar2d.Soar2D;
import soar2d.World;

public class Tank extends Player {
	/**
	 * missile count
	 */
	protected int missiles;
	/**
	 * energy count
	 */
	protected int energy;
	/**
	 * health count
	 */
	protected int health;
	/**
	 * true if shields up
	 */
	protected boolean shieldsUp;

	public Tank( PlayerConfig playerConfig) {
		super(playerConfig);
		
		reset();
	}
	
	public int getMissiles() {
		return missiles;
	}
	/**
	 * @param missiles the new missile count
	 * @param comment why the change happened, keep it very brief
	 * 
	 * set the missiles to a specific value, the comment goes in the log
	 */
	public void setMissiles(int missiles, String comment) {
		this.missiles = missiles;
		if (comment != null) {
			logger.info(getName() + " missiles set to: " + Integer.toString(this.missiles) + " (" + comment + ")");
		} else {
			logger.info(getName() + " missiles set to: " + Integer.toString(this.missiles));
		}
	}
	/**
	 * @param delta the change in missiles
	 * @param comment why the change happened, keep it very brief
	 * 
	 * this is a handy function for changing missiles. puts a message in the log and 
	 * why the change happened
	 */
	public void adjustMissiles(int delta, String comment) {
		int previous = this.missiles;
		this.missiles += delta;
		if (comment != null) {
			logger.info(getName() + " missiles: " + Integer.toString(previous) + " -> " + Integer.toString(this.missiles) + " (" + comment + ")");
		} else {
			logger.info(getName() + " missiles: " + Integer.toString(previous) + " -> " + Integer.toString(this.missiles));
		}
	}

	public int getEnergy() {
		return energy;
	}
	/**
	 * @param energy the new energy count
	 * @param comment why the change happened, keep it very brief
	 * 
	 * set the energy to a specific value, the comment goes in the log
	 */
	public void setEnergy(int energy, String comment) {
		// Bring down shields if out of energy

		this.energy = energy;
		if (comment != null) {
			logger.info(getName() + " energy set to: " + Integer.toString(this.energy) + " (" + comment + ")");
		} else {
			logger.info(getName() + " energy set to: " + Integer.toString(this.energy));
		}
	}
	/**
	 * @param delta the change in energy
	 * @param comment why the change happened, keep it very brief
	 * 
	 * this is a handy function for changing energy. puts a message in the log and 
	 * why the change happened
	 */
	public void adjustEnergy(int delta, String comment) {
		// Bring down shields if out of energy

		int previous = this.energy;
		this.energy += delta;
		if (comment != null) {
			logger.info(getName() + " energy: " + Integer.toString(previous) + " -> " + Integer.toString(this.energy) + " (" + comment + ")");
		} else {
			logger.info(getName() + " energy: " + Integer.toString(previous) + " -> " + Integer.toString(this.energy));
		}
	}

	public int getHealth() {
		return health;
	}
	/**
	 * @param health the new missile count
	 * @param comment why the change happened, keep it very brief
	 * 
	 * set the health to a specific value, the comment goes in the log
	 */
	public void setHealth(int health, String comment) {
		this.health = health;
		if (comment != null) {
			logger.info(getName() + " health set to: " + Integer.toString(this.health) + " (" + comment + ")");
		} else {
			logger.info(getName() + " health set to: " + Integer.toString(this.health));
		}
	}
	/**
	 * @param delta the change in health
	 * @param comment why the change happened, keep it very brief
	 * 
	 * this is a handy function for changing health. puts a message in the log and 
	 * why the change happened
	 */
	public void adjustHealth(int delta, String comment) {
		int previous = this.health;
		this.health += delta;
		if (comment != null) {
			logger.info(getName() + " health: " + Integer.toString(previous) + " -> " + Integer.toString(this.health) + " (" + comment + ")");
		} else {
			logger.info(getName() + " health: " + Integer.toString(previous) + " -> " + Integer.toString(this.health));
		}
	}
	
	public boolean shieldsUp() {
		return shieldsUp;
	}

	public void update(World world, java.awt.Point location) {
		moved = (location.x != this.previousLocation.x) || (location.y != this.previousLocation.y);
	}
	
	public MoveInfo getMove() {
		if (Soar2D.config.graphical == false) {
			return new MoveInfo();
		}
		MoveInfo move = Soar2D.wm.getHumanMove(this.getColor());
		return move;
	}
	
	public void reset() {
		super.reset();
		
		if (playerConfig.hasMissiles()) {
			this.missiles = playerConfig.getMissiles();
		} else {
			this.missiles = Soar2D.config.kDefaultMissiles;
		}
		if (playerConfig.hasHealth()) {
			this.health = playerConfig.getHealth();
		} else {
			this.health = Soar2D.config.kDefaultHealth;
		}
		if (playerConfig.hasEnergy()) {
			this.energy = playerConfig.getEnergy();
		} else {
			this.energy = Soar2D.config.kDefaultEnergy;
		}
		
		shieldsUp = false;
	}
	
	public void setShields(boolean setting) {
		shieldsUp = setting;
	}
	
	public void shutdown() {
		
	}
}
