package tanksoar;

import simulation.*;
import utilities.*;
import sml.*;

public class Tank  extends WorldEntity {
	private final static String kMoveID = "move";
	private final static String kDirectionID = "direction";
	private final static String kBackwardID = "backward";
	private final static String kForwardID = "forward";
	private final static String kLeftID = "left";
	private final static String kRightID = "right";
	private final static String kFireID = "fire";
	private final static String kRadarID = "radar";
	private final static String kSwitchID = "switch";
	//private final static String kOff = "off";
	private final static String kOn = "on";
	private final static String kRadarPowerID = "radar-power";
	private final static String kSettingID = "setting";
	private final static String kShieldsID = "shields";
	private final static String kRotateID = "rotate";

	private TankSoarCell[][] radarCells = new TankSoarCell[kRadarWidth][kRadarHeight];

	public class MoveInfo {
		boolean move;
		int moveDirection;
		
		boolean rotate;
		String rotateDirection;
		
		boolean fire;
		
		boolean radar;
		boolean radarSwitch;
		
		boolean radarPower;
		int radarPowerSetting;
		
		boolean shields;
		boolean shieldsSetting;
		
		public MoveInfo() {
			reset();
		}
		
		public void reset() {
			move = rotate = fire = radar = radarPower = shields = false;
		}
	}
	
	public final static int kRadarWidth = 3;
	public final static int kRadarHeight = 14;
	
	private final static int kSheildEnergyUsage = 20;
	private final static int kMissileHealthDamage = 400;
	private final static int kMissileEnergyDamage = 250;
	private final static int kCollisionHealthDamage = 100;
	
	private final static int kInitialEnergy = 1000;
	private final static int kInitialHealth = 1000;
	private final static int kInitialMissiles = 15;
	
	private final static int kMaximumEnergy = 1000;
	private final static int kMaximumHealth = 1000;
	
	private RelativeDirections m_RD = new RelativeDirections();
	private TankSoarWorld m_World;

	private String m_InitialFacing;
	private MapPoint m_InitialLocation;
	
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

	public Tank(Agent agent, String productions, String color, MapPoint location, String facing, TankSoarWorld world) {
		super(agent, productions, color, location);
		
		m_World = world;
		
		if (facing == null) {			
			facing = WorldEntity.kNorth;
		}
		m_InitialFacing = facing;		
		m_InitialLocation = location;
	
		m_LastMove = new MoveInfo();
		m_ILM = new InputLinkManager(m_World, this);
		
		reset();
	}
	
	public MapPoint getInitialLocation() {
		return m_InitialLocation;
	}
	
	public void reset() {
		m_RadarPower = 0;
		m_RadarDistance = 0;
		m_SmellDistance = 0;
		m_RadarSwitch = false;
		m_RWaves = 0;
		m_Missiles = kInitialMissiles;
		m_ShieldStatus = false;
		m_Health = kInitialHealth;
		m_Energy = kInitialEnergy;
				
		m_LastMove.reset();	
		
		m_Facing = m_InitialFacing;
		setFacingInt();
		m_RD.calculate(getFacingInt());
		
		clearRadar();
		
		m_ILM.clear();
	}
	
	public boolean getRadarStatus() {
		return m_RadarSwitch;
	}
	
	public int getRadarSetting() {
		return m_RadarPower;
	}
	
	// TODO: possibly move calculation in to this class
	void setSmellDistance(int distance) {
		m_SmellDistance = distance;
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
	
	public void readOutputLink() {
		m_LastMove.reset();
		
		int numberOfCommands = m_Agent.GetNumberCommands();
		if (numberOfCommands == 0) {
			m_Logger.log(getName() + " issued no command.");
			return;
		}
		
		for (int i = 0; i < numberOfCommands; ++i) {
			Identifier commandId = m_Agent.GetCommand(i);
			String commandName = commandId.GetAttribute();

			if (commandName.equalsIgnoreCase(kMoveID)) {
				
				String moveDirection = commandId.GetParameterValue(kDirectionID);
				if (moveDirection == null) {
					m_Logger.log(getName() + ": null move direction.");
					continue;
				}
				
				if (moveDirection.equalsIgnoreCase(kForwardID)) {
					m_LastMove.moveDirection = m_RD.forward;
				} else if (moveDirection.equalsIgnoreCase(kBackwardID)) {
					m_LastMove.moveDirection = m_RD.backward;
				} else if (moveDirection.equalsIgnoreCase(kLeftID)) {
					m_LastMove.moveDirection = m_RD.left;
				} else if (moveDirection.equalsIgnoreCase(kRightID)) {
					m_LastMove.moveDirection = m_RD.right;
				} else {
					m_Logger.log(getName() + ": illegal move direction: " + moveDirection);
					continue;
				}
				m_LastMove.move = true;
				
			} else if (commandName.equalsIgnoreCase(kFireID)) {
		 		if (m_Missiles > 0) {
		 			m_LastMove.fire = true;
		 		} else {
					m_Logger.log(getName() + ": Attempted to fire missle with no missiles.");
					continue;
				}
				// Weapon ignored
				
			} else if (commandName.equalsIgnoreCase(kRadarID)) {
				
				String radarSwitch = commandId.GetParameterValue(kSwitchID);
				if (radarSwitch == null) {
					m_Logger.log(getName() + ": null radar switch.");
					continue;
				}
				m_LastMove.radar = true;
				m_LastMove.radarSwitch = radarSwitch.equalsIgnoreCase(kOn) ? true : false;  
				
			} else if (commandName.equalsIgnoreCase(kRadarPowerID)) {
				
				String powerValue = commandId.GetParameterValue(kSettingID);
				if (powerValue == null) {
					m_Logger.log(getName() + ": null radar power value.");
					continue;
				}
				
				try {
					m_LastMove.radarPowerSetting = Integer.decode(powerValue).intValue();
				} catch (NumberFormatException e) {
					m_Logger.log(getName() + ": Unable to parse radar power setting " + powerValue + ": " + e.getMessage());
					continue;
				}
				m_LastMove.radarPower = true;
				
			} else if (commandName.equalsIgnoreCase(kShieldsID)) {
				
				String shieldsSetting = commandId.GetParameterValue(kSwitchID);
				if (shieldsSetting == null) {
					m_Logger.log(getName() + ": null shields setting.");
					continue;
				}
				m_LastMove.shields = true;
				m_LastMove.shieldsSetting = shieldsSetting.equalsIgnoreCase(kOn) ? true : false; 
				
			} else if (commandName.equalsIgnoreCase(kRotateID)) {
				
				m_LastMove.rotateDirection = commandId.GetParameterValue(kDirectionID);
				if (m_LastMove.rotateDirection == null) {
					m_Logger.log(getName() + ": null rotation direction.");
					continue;
				}
				
				m_LastMove.rotate = true;
				
				// Rotation must be handled pronto and never fails.
				if (m_LastMove.rotate) {
					rotate(m_LastMove.rotateDirection);
				}
				
			} else {
				m_Logger.log(getName() + ": Unknown command: " + commandName);
				continue;
			}
			commandId.AddStatusComplete();
		}
		
    	m_Agent.ClearOutputLinkChanges();
		m_Agent.Commit();
		
		// Do not allow a move if we rotated.
		if (m_LastMove.rotate) {
			if (m_LastMove.move) {
				m_Logger.log("Tried to move with a rotation, rotating only.");
				m_LastMove.move = false;
			}
		}
	}
	
	public Integer getMove() {
		m_LastMove.reset();
		m_RWaves = 0;
		
		if (m_Agent.GetNumberCommands() == 0) {
			m_Logger.log(getName() + " issued no command.");
			return null;
		}
		
		for (int i = 0; i < m_Agent.GetNumberCommands(); ++i) {
		
			Identifier commandId = m_Agent.GetCommand(i);
			String commandName = commandId.GetAttribute();

			if (commandName.equalsIgnoreCase(kMoveID)) {
				m_LastMove.move = true;
				
				if (commandId.GetParameterValue(kDirectionID).equalsIgnoreCase(kForwardID)) {
					m_LastMove.moveDirection = m_RD.forward;
				} else if (commandId.GetParameterValue(kDirectionID).equalsIgnoreCase(kBackwardID)) {
					m_LastMove.moveDirection = m_RD.backward;
				} else if (commandId.GetParameterValue(kDirectionID).equalsIgnoreCase(kLeftID)) {
					m_LastMove.moveDirection = m_RD.left;
				} else if (commandId.GetParameterValue(kDirectionID).equalsIgnoreCase(kRightID)) {
					m_LastMove.moveDirection = m_RD.right;
				} else {
					m_Logger.log("Assert");
				}
				
			} else if (commandName.equalsIgnoreCase(kFireID)) {
		 		if (m_Missiles > 0) {
		 			m_LastMove.fire = true;
		 		} else {
					m_Logger.log(getName() + ": Attempted to fire missle with no missiles.");
				}
				// Weapon ignored
				
			} else if (commandName.equalsIgnoreCase(kRadarID)) {
				m_LastMove.radar = true;
				m_LastMove.radarSwitch = commandId.GetParameterValue(kSwitchID).equalsIgnoreCase(kOn) ? true : false;  
				
			} else if (commandName.equalsIgnoreCase(kRadarPowerID)) {
				String powerValue = commandId.GetParameterValue(kSettingID);
				if (powerValue != null) {
					try {
						m_LastMove.radarPowerSetting = Integer.decode(powerValue).intValue();
					} catch (NumberFormatException e) {
						m_Logger.log(getName() + ": Unable to parse radar power setting " + powerValue + ": " + e.getMessage());
					}
					m_LastMove.radarPower = true;
				} else {
					m_Logger.log(getName() + ": Radar power setting is null.");
				}
				
			} else if (commandName.equalsIgnoreCase(kShieldsID)) {
				m_LastMove.shields = true;
				m_LastMove.shieldsSetting = commandId.GetParameterValue(kSwitchID).equalsIgnoreCase(kOn) ? true : false; 
				
			} else if (commandName.equalsIgnoreCase(kRotateID)) {
				m_LastMove.rotate = true;
				m_LastMove.rotateDirection = commandId.GetParameterValue(kDirectionID);
				
				// Rotation must be handled pronto and never fails.
				if (m_LastMove.rotate) {
					rotate(m_LastMove.rotateDirection);
				}
				
			} else {
				m_Logger.log(getName() + ": Unknown command: " + commandName);
				continue;
			}
			commandId.AddStatusComplete();
		}
		
    	m_Agent.ClearOutputLinkChanges();
		m_Agent.Commit();
		
		// Do not allow a move if we rotated.
		if (m_LastMove.rotate) {
			if (m_LastMove.move) {
				m_Logger.log("Tried to move with a rotation, rotating only.");
				m_LastMove.move = false;
			}
		}
		return m_LastMove.move ? new Integer(m_LastMove.moveDirection) : null;
	}
	
	public void updateSensors(TankSoarWorld world) {		
		TankSoarCell cell = world.getCell(getLocation());
		
		// Chargers
		if (m_Health > 0) {
			if (cell.isEnergyRecharger()) {
				m_Energy += 250;
				m_Energy = m_Energy > kMaximumEnergy ? kMaximumEnergy : m_Energy;
			}
			if (cell.isHealthRecharger()) {
				m_Health += 250;
				m_Health = m_Health > kMaximumHealth ? kMaximumHealth : m_Health;
			}
		}
		
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
		}
		// Consume shield energy.
		if (m_ShieldStatus) {
			m_Energy -= kSheildEnergyUsage;
		}
	
		// Handle radar.
		// Figure out desired radar power.
		int desiredRadarPower = m_LastMove.radarPower ? m_LastMove.radarPowerSetting : m_RadarPower;
		// Never exceed max power
		desiredRadarPower = desiredRadarPower > Tank.kRadarHeight ? Tank.kRadarHeight : desiredRadarPower;
		// Never exceed current energy usage
		desiredRadarPower = desiredRadarPower > m_Energy ? m_Energy : desiredRadarPower;
		// If the power setting changed, set it.
		if (m_RadarPower != desiredRadarPower) {
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
			m_Energy -= m_RadarPower;
			m_RadarDistance = scan(m_RadarPower);
		} else {
			clearRadar();
			m_RadarDistance = 0;
		}
		
		// Missiles
		if (m_LastMove.fire) {
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
	
	private int scan(int setting) {
		MapPoint location = new MapPoint(getLocation());
		
		int distance = 0;
		for (int i = 0; i <= setting; ++i) {
			distance = i;
			if (scanCells(i, location) == true) {
				// Blocked
				clearCells(i, location);
				break;
			}

			// Update for next distance
			location.travel(forward());
		}
		return distance;
	}
	
	static final int kRadarLeft = 0;
	static final int kRadarCenter = 1;
	static final int kRadarRight = 2;
	
	private boolean scanCells(int distance, MapPoint location) {
		for (int i = 0; i < Tank.kRadarWidth; ++i) {
			// Scan center then left then right
			int position = 0;
			int relativeDirection = 0;
			
			switch (i) {
			case 0:
				position = kRadarCenter;
				relativeDirection = 0;
				break;
			default:
			case 1:
				position = kRadarLeft;
				relativeDirection = left();
				break;
			case 2:
				position = kRadarRight;
				relativeDirection = right();
				break;
			}
			
			radarCells[position][distance] = m_World.getCell(location, relativeDirection);
			if (radarCells[position][distance].containsTank()) {
				radarCells[position][distance].getTank().setRWaves(backward());
			}

			if ((position == kRadarCenter) && radarCells[position][distance].isBlocked()) {
				if ((position != kRadarCenter) || (distance != 0)) {
					return true;
				}
			}
			radarCells[position][distance].setRadarTouch();
		}
		return false;
	}
	
	private void clearCells(int initialDistance, MapPoint location) {
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
		m_ILM.write();
	}
	
	int forward() {
		return m_RD.forward;
	}
	
	int backward() {
		return m_RD.backward;
	}
	
	int left() {
		return m_RD.left;
	}
	
	int right() {
		return m_RD.right;
	}
	
	public void addMissiles(int numberToAdd) {
		m_Missiles += numberToAdd;
	}
	
	public int getMissiles() {
		return m_Missiles;
	}
	
	public int getHealth() {
		return m_Health;
	}
	
	public void hitBy(Tank attacker, TankSoarCell currentCell) {
		// Missile damage
		if (m_ShieldStatus) {
			m_Energy -= kMissileEnergyDamage;
			if (m_Energy < 0) {
				m_Energy = 0;
			}
		} else {
			m_Health -= kMissileHealthDamage;
			if (m_Health < 0) {
				m_Health = 0;
			}
		}

		// Chargers
		if (m_Health > 0) {
			if (currentCell.isEnergyRecharger()) {
				// Instant death
				m_Health = 0;
			}
			if (currentCell.isHealthRecharger()) {
				// Instant death
				m_Health = 0;
			}
		}
		
		if (m_Health <= 0) {
			adjustPoints(TankSoarWorld.kKillPenalty);
			// TODO: multiple missiles!
			attacker.adjustPoints(TankSoarWorld.kKillAward);
		}
	}
	
	public void collide() {
		// Collision damage
		m_Health -= kCollisionHealthDamage;
		if (m_Health < 0) {
			m_Health = 0;
		}
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
	
	public int lastMoveDirection() {
		return m_LastMove.moveDirection;
	}
	
	private void rotate(String direction) {		
		int facing = 0;
		
		if (direction.equalsIgnoreCase(kLeftID)) {
			facing = m_RD.left;
		} else if (direction.equalsIgnoreCase(kRightID)) {
			facing = m_RD.right;
		}
		
		switch (facing) {
		case kNorthInt:
			m_Facing = kNorth;
			break;
		case kSouthInt:
			m_Facing = kSouth;
			break;
		case kEastInt:
			m_Facing = kEast;
			break;
		case kWestInt:
			m_Facing = kWest;
			break;
		}
		
		setFacingInt();
		m_RD.calculate(getFacingInt());
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
