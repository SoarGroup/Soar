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
	
	private RelativeDirections m_RD = new RelativeDirections();
	private TankSoarWorld m_World;

	private String m_InitialFacing;
	private MapPoint m_InitialLocation;
	
	private InputLinkManager m_ILM;
	private MoveInfo m_LastMove;
	private int m_RWaves;
	private Tank m_HitByTank;
	private boolean m_Collision;
	private int m_Missiles;
	private boolean m_ShieldStatus;
	private int m_Health;
	private int m_Energy;
	private int m_RadarPower;
	private boolean m_RadarSwitch;
	private int m_RadarDistance;
	
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
		
		m_HitByTank = null;
		m_Collision = false;
		
		m_ILM.clear();
	}
	
	public boolean getRadarStatus() {
		return m_RadarSwitch;
	}
	
	public int getRadarSetting() {
		// Even though 0 is useful for us, it isn't a legal setting.
		if (m_RadarPower == 0) {
			return 1;
		}
		return m_RadarPower;
	}
	
	void setRadarDistance(int distance) {
		m_RadarDistance = distance;
	}
	
	public int getRadarDistance() {
		return m_RadarDistance;
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
    	
    	// TODO is this necessary?
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
	
	public void update(TankSoarWorld world) {		
		// fire input updated at read time.
		
		TankSoarCell cell = world.getCell(getLocation());
		
		// Missile damage
		if (m_HitByTank != null) {
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
		}
		
		// Collision damage
		if (m_Collision) {
			m_Health -= kCollisionHealthDamage;
			if (m_Health < 0) {
				m_Health = 0;
			}
		}

		m_Collision = false;

		// Chargers
		if (m_Health > 0) {
			if (cell.isEnergyRecharger()) {
				// Check for missile hit
				if (m_HitByTank != null) {
					// Instant death
					m_Health = 0;
				} else {
					m_Energy += 250;
					m_Energy = m_Energy > kInitialEnergy ? kInitialEnergy : m_Energy;
				}
			}
			if (cell.isHealthRecharger()) {
				// Check for missile hit
				if (m_HitByTank != null) {
					// Instant death
					m_Health = 0;
				} else {
					m_Health += 250;
					m_Health = m_Health > kInitialHealth ? kInitialHealth : m_Health;
				}
			}
		}
		
		if (m_Health <= 0) {
			adjustPoints(TankSoarWorld.kKillPenalty);
			if (m_HitByTank != null) {
				m_HitByTank.adjustPoints(TankSoarWorld.kKillAward);
			}
		}
		
		m_HitByTank = null;
		
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
		}
		
		// Missiles
		if (m_LastMove.fire) {
			m_Missiles -= 1;
		}
	
		m_ILM.update();
	}
	
	void update2() {
		m_ILM.updateRWaves();		
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
	
	public void hitBy(Tank hitByTank) {
		m_HitByTank = hitByTank;
	}
	
	public void collide() {
		m_Collision = true;
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
	
	public String getRadarID(int x, int y) {
		return m_ILM.getRadarID(x, y);
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
