package tanksoar;

import java.util.logging.*;

import simulation.*;
import utilities.*;
import sml.*;

public class Tank  extends WorldEntity {
	private static Logger logger = Logger.getLogger("simulation");
	
	private final static String kMoveID = "move";
	private final static String kDirectionID = "direction";
	private final static String kBackwardID = "backward";
	private final static String kForwardID = "forward";
	public final static String kLeftID = "left";
	public final static String kRightID = "right";
	private final static String kFireID = "fire";
	private final static String kRadarID = "radar";
	private final static String kSwitchID = "switch";
	//private final static String kOff = "off";
	private final static String kOn = "on";
	private final static String kRadarPowerID = "radar-power";
	private final static String kSettingID = "setting";
	private final static String kShieldsID = "shields";
	private final static String kRotateID = "rotate";

	private static final int kMissileHitAward = 2;
	private static final int kMissileHitPenalty = -1;
	private static final int kKillAward = 3;
	private static final int kKillPenalty = -2;
	

	private TankSoarCell[][] radarCells = new TankSoarCell[kRadarWidth][kRadarHeight];

	public final static int kRadarWidth = 3;
	public final static int kRadarHeight = 15;
	
	private final static int kSheildEnergyUsage = 20;
	private final static int kMissileHealthDamage = 400;
	private final static int kMissileEnergyDamage = 250;
	private final static int kCollisionHealthDamage = 100;
	
	private final static int kMaximumEnergy = 1000;
	private final static int kMaximumHealth = 1000;
	
	private TankSoarWorld m_World;

	private int m_InitialFacing;
	private java.awt.Point m_InitialLocation;
	
	private InputLinkManager m_ILM;
	private MoveInfo m_LastMove;
	private int m_RWaves;
	private int m_Missiles;
	private boolean m_ShieldStatus;
	private int m_Health;
	private int m_Energy;
	private int m_RadarPower;
	private boolean m_RadarSwitch;
	private int m_RadarDistance;
	private int m_SmellDistance;
	private String m_SmellColor;
	
	private int m_InitialEnergy = 1000;
	private int m_InitialHealth = 1000;
	private int m_InitialMissiles = 15;
	

	public Tank(Agent agent, String productions, String color, java.awt.Point location, String facing, int energy, int health, int missiles, TankSoarWorld world) {
		super(agent, productions, color, location);
		
		m_World = world;
		
		if (facing == null) {	
			// FIXME: should be random
			facing = Direction.kNorthString;
		}
		// FIXME: should set to null if was null coming in
		m_InitialFacing = Direction.getInt(facing);		
		m_InitialLocation = location;
		
		if (energy != -1) {
			m_InitialEnergy = energy;
		}
		if (health != -1) {
			m_InitialHealth = health;
		}
		if (missiles != -1) {
			m_InitialMissiles = missiles;
		}
	
		m_LastMove = new MoveInfo();
		if (m_Agent != null) {
			m_ILM = new InputLinkManager(m_World, this);
		}
		
		logger.info("Created tank: " + getName());
		
		reset();
	}
	
	public boolean equals(Tank tank) {
		if (getName().equals(tank.getName())) {
			return true;
		}
		return false;
	}
	
	public java.awt.Point getInitialLocation() {
		return m_InitialLocation;
	}
	
	public void reset() {
		m_RadarPower = 0;
		m_RadarDistance = 0;
		m_SmellDistance = 0;
		m_SmellColor = null;
		m_RadarSwitch = false;
		m_RWaves = 0;
		m_Missiles = m_InitialMissiles;
		m_ShieldStatus = false;
		m_Health = m_InitialHealth;
		m_Energy = m_InitialEnergy;
				
		m_LastMove.reset();	
		
		m_FacingInt = m_InitialFacing;
		
		clearRadar();
		
		if (m_Agent != null) {
			m_ILM.clear();
		}
		
		logger.info(getName() + ": reset: (facing: " + m_InitialFacing 
				+ ")(missiles: " + Integer.toString(m_InitialMissiles)
				+ ")(health: " + Integer.toString(m_InitialHealth)
				+ ")(energy: " + Integer.toString(m_InitialEnergy)
				+ ")");
	}
	
	public boolean getRadarStatus() {
		return m_RadarSwitch;
	}
	
	public int getRadarSetting() {
		return m_RadarPower;
	}
	
	// TODO: possibly move calculation in to this class
	void setSmell(int distance, String color) {
		m_SmellDistance = distance;
		m_SmellColor = color;
	}
	
	public TankSoarCell[][] getRadarCells() {
		return radarCells;
	}
	
	public int getRadarDistance() {
		return m_RadarDistance;
	}
	
	public int getSmellDistance() {
		return m_SmellDistance;
	}
	
	public String getSmellColor() {
		return m_SmellColor;
	}
	
	public void humanInput(MoveInfo move) {
		assert move != null;
		
		m_LastMove.reset();
		m_LastMove = move;
		m_RWaves = 0;
		
		// Must check missile count now
		if (m_LastMove.fire) {
			if (m_Missiles <= 0) {
				logger.fine(getName() + ": Attempted to fire missle with no missiles.");	
				m_LastMove.fire = false;
			}
		}

		// Shields must be handled pronto
		if (m_LastMove.shields) {
			handleShields();
		}
		
		if (m_LastMove.rotate) {
			rotate(m_LastMove.rotateDirection);
			// Do not allow a move if we rotated.
			if (m_LastMove.move) {
				logger.fine(getName() + ": Tried to move with a rotation, rotating only.");
				m_LastMove.move = false;
			}
		}
		logger.info(getName() + " (human) at " + getLocation().toString() +", world count: " + Integer.toString(m_World.getWorldCount()) + ", move: " + m_LastMove.toString());
	}
	
	public void readOutputLink() {
		m_LastMove.reset();
		m_RWaves = 0;
		
		assert m_Agent != null;
		int numberOfCommands = m_Agent.GetNumberCommands();
		if (numberOfCommands == 0) {
			logger.fine(getName() + ": world count: " + Integer.toString(m_World.getWorldCount()) + ", issued no command.");
			return;
		}
		Identifier moveId = null;
		
		for (int i = 0; i < numberOfCommands; ++i) {
			Identifier commandId = m_Agent.GetCommand(i);
			String commandName = commandId.GetAttribute();

			if (commandName.equalsIgnoreCase(kMoveID)) {
				if (m_LastMove.move == true) {
					logger.fine(getName() + ": Extra move commands detected.");
					commandId.AddStatusError();
					continue;
				}

				String moveDirection = commandId.GetParameterValue(kDirectionID);
				if (moveDirection == null) {
					logger.fine(getName() + ": null move direction.");
					commandId.AddStatusError();
					continue;
				}
				
				if (moveDirection.equalsIgnoreCase(kForwardID)) {
					m_LastMove.moveDirection = m_FacingInt;
				} else if (moveDirection.equalsIgnoreCase(kBackwardID)) {
					m_LastMove.moveDirection = Direction.backwardOf[this.m_FacingInt];
				} else if (moveDirection.equalsIgnoreCase(kLeftID)) {
					m_LastMove.moveDirection = Direction.leftOf[this.m_FacingInt];
				} else if (moveDirection.equalsIgnoreCase(kRightID)) {
					m_LastMove.moveDirection = Direction.rightOf[this.m_FacingInt];
				} else {
					logger.fine(getName() + ": illegal move direction: " + moveDirection);
					commandId.AddStatusError();
					continue;
				}
				moveId = commandId;
				m_LastMove.move = true;
				
			} else if (commandName.equalsIgnoreCase(kFireID)) {
				if (m_LastMove.fire == true) {
					logger.fine(getName() + ": Extra fire commands detected.");
					commandId.AddStatusError();
					continue;
				}
				
		 		if (m_Missiles > 0) {
		 			m_LastMove.fire = true;
		 		} else {
					logger.fine(getName() + ": Attempted fire with no missles.");
					commandId.AddStatusError();
					continue;
				}
				// Weapon ignored
				
			} else if (commandName.equalsIgnoreCase(kRadarID)) {
				if (m_LastMove.radar == true) {
					logger.fine(getName() + ": Extra radar commands detected.");
					commandId.AddStatusError();
					continue;
				}
				
				String radarSwitch = commandId.GetParameterValue(kSwitchID);
				if (radarSwitch == null) {
					logger.fine(getName() + ": null radar switch.");
					commandId.AddStatusError();
					continue;
				}
				m_LastMove.radar = true;
				m_LastMove.radarSwitch = radarSwitch.equalsIgnoreCase(kOn) ? true : false;  
				
			} else if (commandName.equalsIgnoreCase(kRadarPowerID)) {
				if (m_LastMove.radarPower == true) {
					logger.fine(getName() + ": Extra radar power commands detected.");
					commandId.AddStatusError();
					continue;
				}
				
				String powerValue = commandId.GetParameterValue(kSettingID);
				if (powerValue == null) {
					logger.fine(getName() + ": null radar power value.");
					commandId.AddStatusError();
					continue;
				}
				
				try {
					m_LastMove.radarPowerSetting = Integer.decode(powerValue).intValue();
				} catch (NumberFormatException e) {
					logger.fine(getName() + ": Unable to parse radar power setting " + powerValue + ": " + e.getMessage());
					commandId.AddStatusError();
					continue;
				}
				m_LastMove.radarPower = true;
				
			} else if (commandName.equalsIgnoreCase(kShieldsID)) {
				if (m_LastMove.shields == true) {
					logger.fine(getName() + ": Extra shield commands detected.");
					commandId.AddStatusError();
					continue;
				}
				
				String shieldsSetting = commandId.GetParameterValue(kSwitchID);
				if (shieldsSetting == null) {
					logger.fine(getName() + ": null shields setting.");
					commandId.AddStatusError();
					continue;
				}
				m_LastMove.shields = true;
				m_LastMove.shieldsSetting = shieldsSetting.equalsIgnoreCase(kOn) ? true : false; 
				
				// Shields must be handled pronto
				handleShields();
				
			} else if (commandName.equalsIgnoreCase(kRotateID)) {
				if (m_LastMove.rotate == true) {
					logger.fine(getName() + ": Extra rotate commands detected.");
					commandId.AddStatusError();
					continue;
				}
				
				m_LastMove.rotateDirection = commandId.GetParameterValue(kDirectionID);
				if (m_LastMove.rotateDirection == null) {
					logger.fine(getName() + ": null rotation direction.");
					commandId.AddStatusError();
					continue;
				}
				
				m_LastMove.rotate = true;
				
				// Rotation must be handled pronto and never fails.
				if (m_LastMove.rotate) {
					rotate(m_LastMove.rotateDirection);
				}
				
			} else {
				logger.warning(getName() + ": Unknown command: " + commandName);
				commandId.AddStatusError();
				continue;
			}
			commandId.AddStatusComplete();
		}
		
    	m_Agent.ClearOutputLinkChanges();
		m_Agent.Commit();
		
		// Do not allow a move if we rotated.
		if (m_LastMove.rotate) {
			if (m_LastMove.move) {
				logger.fine("Tried to move with a rotation, rotating only.");
				assert moveId != null;
				moveId.AddStatusError();
				moveId = null;
				m_LastMove.move = false;
			}
		}
		logger.info(getName() + " at " + getLocation().toString() +", world count: " + Integer.toString(m_World.getWorldCount()) + ", move: " + m_LastMove.toString());
	}
	
	private void handleShields() {
		// Handle shields.
		boolean desiredShieldStatus = m_LastMove.shields ? m_LastMove.shieldsSetting : m_ShieldStatus;
		boolean enoughPowerForShields = m_Energy >= kSheildEnergyUsage;
		// If the agent wants to turn the shield on
		if (!m_ShieldStatus && desiredShieldStatus) {
			// Turn the shield on if there is enough power.
			if (enoughPowerForShields) {
				m_ShieldStatus = true;
			}
		}
		// If the agent wants the shield off or there isn't enough power
		if ((m_ShieldStatus && !desiredShieldStatus) || !enoughPowerForShields) {
			// Turn the shield off.
			m_ShieldStatus = false;
			if (!enoughPowerForShields) {
				logger.fine(getName() + ": Not enough power for shields.");
			}
		}
	}
	
	public void updateSensors(TankSoarWorld world) {		
		TankSoarCell cell = world.getCell(getLocation());
		
		// Chargers
		if (m_Health > 0) {
			if (cell.isEnergyRecharger()) {
				adjustEnergy(250, "energy recharger");
			}
			if (cell.isHealthRecharger()) {
				adjustHealth(250, "health recharger");
			}
		}
		
		// Consume shield energy.
		if (m_ShieldStatus) {
			boolean enoughPowerForShields = m_Energy >= kSheildEnergyUsage;
			if (enoughPowerForShields) {
				adjustEnergy(kSheildEnergyUsage * -1, "shield usage");
			} else {
				logger.fine(getName() + ": Not enough power for shields.");
				m_ShieldStatus = false;
			}
		}
		
		// Handle radar.
		// Figure out desired radar power.
		int desiredRadarPower = m_LastMove.radarPower ? m_LastMove.radarPowerSetting : m_RadarPower;
		// Never exceed max power
		desiredRadarPower = desiredRadarPower >= Tank.kRadarHeight ? Tank.kRadarHeight - 1 : desiredRadarPower;
		// Never exceed current energy usage
		if (desiredRadarPower > m_Energy) {
			logger.fine(getName() + ": Radar power limited by available energy: " + Integer.toString(m_Energy));
			desiredRadarPower =  m_Energy;
		}
		// If the power setting changed, set it.
		if (m_RadarPower != desiredRadarPower) {
			logger.fine(getName() + " radar power: " + Integer.toString(m_RadarPower) + " -> " + Integer.toString(desiredRadarPower));
			m_RadarPower = desiredRadarPower;
		}
		
		// Figure out desired radar status
		boolean desiredRadarStatus = m_LastMove.radar ? m_LastMove.radarSwitch : m_RadarSwitch;
		// If the agent wants to turn the radar on
		if (!m_RadarSwitch && desiredRadarStatus) {
			// Turn the radar on.
			if (m_RadarPower > 0) {
				m_RadarSwitch = true;
			}
		}
		// If the agent wants to turn the radar off or there isn't enough power
		if ((m_RadarSwitch && !desiredRadarStatus) || (m_RadarPower == 0)) {
			// Turn the radar off.
			m_RadarSwitch = false;
		}
		// Consume radar energy.
		if (m_RadarSwitch) {
			adjustEnergy(m_RadarPower * -1, "radar usage");
			m_RadarDistance = scan(m_RadarPower);
		} else {
			clearRadar();
			m_RadarDistance = 0;
		}
		
		// Missiles
		if (m_LastMove.fire) {
			logger.fine(getName() + ": Consuming missile.");
			m_Missiles -= 1;
		}
	
	}
	
	private void clearRadar() {
		for (int i = 0; i < kRadarWidth; ++i) {
			for (int j = 0; j < kRadarHeight; ++j) {
				radarCells[i][j] = null;
			}
		}
	}
	
	static final int kRadarLeft = 0;
	static final int kRadarCenter = 1;
	static final int kRadarRight = 2;
	
	private int scan(int setting) {
		int distance;
		int scanX = this.getLocation().x;
		int scanY = this.getLocation().y;
		boolean stop = false;
		
		for (distance = 0; distance <= setting; ++distance, scanX += Direction.xDelta[m_FacingInt], scanY += Direction.yDelta[m_FacingInt]) {
			for (int i = 0; i < Tank.kRadarWidth; ++i) {
				// Scan center then left then right
				int position = 0;
				int relativeX = 0;
				int relativeY = 0;
				
				switch (i) {
				case 0:
					if (distance == 0) {
						continue;
					}
					position = kRadarCenter;
					break;
				default:
				case 1:
					position = kRadarLeft;
					relativeX += Direction.xDelta[Direction.leftOf[m_FacingInt]];
					relativeY += Direction.yDelta[Direction.leftOf[m_FacingInt]];
					break;
				case 2:
					position = kRadarRight;
					relativeX += Direction.xDelta[Direction.rightOf[m_FacingInt]];
					relativeY += Direction.yDelta[Direction.rightOf[m_FacingInt]];
					break;
				}
				
				radarCells[position][distance] = m_World.getCell(scanX + relativeX, scanY + relativeY);
				if (radarCells[position][distance].containsTank()) {
					logger.fine(getName() + ": Radar waves from " 
							+ Direction.stringOf[Direction.backwardOf[this.m_FacingInt]] 
							+ " hitting tank " + radarCells[position][distance].getTank().getName());
					radarCells[position][distance].getTank().setRWaves(Direction.backwardOf[this.m_FacingInt]);
				}
				
				if ((position == kRadarCenter) && radarCells[position][distance].isBlocked()) {
					stop = true;
					break;
				}
				radarCells[position][distance].setRadarTouch();
			}
			
			if (stop) {
				logger.fine(getName() + ": Radar scan blocked at distance " + Integer.toString(distance));
				break;
			}
		}
		clearCells(distance);
		return distance;
	}
	
	private void clearCells(int initialDistance) {
		for (int j = initialDistance; j < Tank.kRadarHeight; ++j) {
			for (int i = 0; i < Tank.kRadarWidth; ++i) {
				if ((i == 1) && (j == initialDistance)) {
					// skip first center
					continue;
				}
				radarCells[i][j] = null;
			}
		}
	}
	
	public void writeInputLink() {
		if (m_Agent != null) {
			m_ILM.write();
		}
	}
	
	public void addMissiles(int numberToAdd) {
		logger.info(getName() + ": picked up " + Integer.toString(numberToAdd) + " missiles.");
		m_Missiles += numberToAdd;
	}
	
	public int getMissiles() {
		return m_Missiles;
	}
	
	public int getHealth() {
		return m_Health;
	}
	
	public void hitBy(Integer[] missileIDs) {
		Tank[] owners = new Tank[missileIDs.length];
		
		String output = getName() + ": hit by missile(s): ";
		for (int i = 0; i < missileIDs.length; ++i) {
			owners[i] = m_World.getMissileByID(missileIDs[i].intValue()).getOwner();
			output += missileIDs[i].toString() + " (" + owners[i].getName() + ")";
		}
		logger.info(output);
		
		if (m_ShieldStatus) {
			adjustEnergy(kMissileEnergyDamage * -1 * missileIDs.length, "hit with shields up");
		} else {
			adjustHealth(kMissileHealthDamage * -1 * missileIDs.length, "hit");
			for (int i = 0; i < owners.length; ++i) {
				assert !owners[i].equals(this);
				owners[i].adjustPoints(kMissileHitAward, "hit award");
				adjustPoints(kMissileHitPenalty, "hit penalty");
			}
		}
		
		if (m_World.getCell(getLocation()).isEnergyRecharger() || m_World.getCell(getLocation()).isHealthRecharger()) {
			logger.info(getName() + ": hit on charger");
			adjustHealth(m_Health * -1, "hit on charger");
		}
		
		if (m_Health <= 0) {
			logger.info(getName() + ": fragged");
			adjustPoints(kKillPenalty, "kill penalty");
			for (int i = 0; i < owners.length; ++i) {
				assert !owners[i].equals(this);
				boolean duplicate = false;
				for (int j = i - 1; j >= 0; --j) {
					// remove duplicates
					if (owners[i].equals(owners[j])) {
						duplicate = true;
					}
				}
				if (!duplicate) {
					owners[i].adjustPoints(kKillAward, "kill award");
				}
			}
		}
	}
	
	public void adjustEnergy(int delta, String comment) {
		int previous = m_Energy;
		m_Energy += delta;
		if (m_Energy < 0) {
			m_Energy = 0;
		} else if (m_Energy > kMaximumEnergy) {
			m_Energy = kMaximumEnergy;
		}			
		logger.info(getName() + ": energy: " + Integer.toString(previous) + " -> " + Integer.toString(m_Energy) + " (" + comment + ")");
	}
	
	public void adjustHealth(int delta, String comment) {
		int previous = m_Health;
		m_Health += delta;
		if (m_Health < 0) {
			m_Health = 0;
		} else if (m_Health > kMaximumHealth) {
			m_Health = kMaximumHealth;
		}
		logger.info(getName() + ": health: " + Integer.toString(previous) + " -> " + Integer.toString(m_Health) + " (" + comment + ")");
	}
	
	public void collide() {
		// Collision damage
		adjustHealth(kCollisionHealthDamage * -1, "collision");
		m_LastMove.move = false;
	}
	public int getEnergy() {
		return m_Energy;
	}
	
	public boolean getShieldStatus() {
		return m_ShieldStatus;
	}
	
	public boolean recentlyMovedOrRotated() {
		return m_LastMove.move || m_LastMove.rotate;
	}
	
	public boolean recentlyMoved() {
		return m_LastMove.move;
	}
	
	public boolean recentlyRotated() {
		return m_LastMove.rotate;
	}
	
	public int getLastMoveDirection() {
		return m_LastMove.moveDirection;
	}
	
	private void rotate(String direction) {	
		if (direction.equalsIgnoreCase(kLeftID)) {
			m_FacingInt = Direction.leftOf[m_FacingInt];
		} else if (direction.equalsIgnoreCase(kRightID)) {
			m_FacingInt = Direction.rightOf[m_FacingInt];
		} else {
			assert false;
		}
	}
	
	public boolean firedMissile() {
		return m_LastMove.fire;
	}
	
	public int getRWaves() {
		return m_RWaves;
	}
	
	void setRWaves(int fromDirection) {
		m_RWaves |= fromDirection;
	}
	
}
