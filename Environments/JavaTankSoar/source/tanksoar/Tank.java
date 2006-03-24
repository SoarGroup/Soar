package tanksoar;

import java.util.*;

import simulation.*;
import utilities.*;
import sml.*;

public class Tank  extends WorldEntity {
	
	private final static String kBlockedID = "blocked";
	private final static String kClockID = "clock";
	private final static String kColorID = "color";
	private final static String kDirectionID = "direction";
	private final static String kDistanceID = "distance";
	public final static String kEnergyID = "energy";
	private final static String kEnergyRechargerID = "energyrecharger";
	private final static String kFireID = "fire";
	public final static String kHealthID = "health";
	private final static String kHealthRechargerID = "healthrecharger";
	private final static String kIncomingID = "incoming";
	public final static String kMissilesID = "missiles";
	private final static String kMoveID = "move";
	private final static String kMyColorID = "my-color";
	public final static String kObstacleID = "obstacle";
	public final static String kOpenID = "open";
	public final static String kTankID = "tank";
	private final static String kRadarID = "radar";
	private final static String kRadarDistanceID = "radar-distance";
	private final static String kRadarPowerID = "radar-power";
	private final static String kRadarSettingID = "radar-setting";
	private final static String kRadarStatusID = "radar-status";
	private final static String kRandomID = "random";
	private final static String kResurrectID = "resurrect";
	private final static String kRotateID = "rotate";
	private final static String kRWavesID = "rwaves";
	private final static String kShieldsID = "shields";
	private final static String kSettingID = "setting";
	private final static String kShieldStatusID = "shield-status";
	private final static String kSmellID = "smell";
	private final static String kSoundID = "sound";
	private final static String kSwitchID = "switch";
	private final static String kXID = "x";
	private final static String kYID = "y";
	
	private final static String kBackwardID = "backward";
	private final static String kForwardID = "forward";
	private final static String kLeftID = "left";
	private final static String kRightID = "right";
	private final static String kSilentID = "silent";
	private final static String kPositionID = "position";
	private final static String kCenterID = "center";

	public final static String kEast = "east";
	public final static String kNorth = "north";
	public final static String kSouth = "south";
	public final static String kWest = "west";
	
	private final static String kYes = "yes";
	private final static String kNo = "no";
	
	private final static String kOff = "off";
	private final static String kOn = "on";
	
	private final static String kNone = "none";
	
	public final static int kRadarWidth = 3;
	public final static int kRadarHeight = 14;
	
	private final static int kSheildEnergyUsage = 20;
	private final static int kMissileHealthDamage = 400;
	private final static int kMissileEnergyDamage = 250;
	
	public class Radar {
		//0[left  0][left  1][left  2]
		//1[tank  0][center1][center2]-->facing
		//2[right 0][right 1][right 2]
		private static final int kRadarLeft = 0;
		private static final int kRadarCenter = 1;
		private static final int kRadarRight = 2;
		
		private Identifier[][] cellIDs = new Identifier[kRadarWidth][kRadarHeight];
		private StringElement[][] tankColors = new StringElement[kRadarWidth][kRadarHeight];
		
		private StringElement m_RadarStatusWME;
		private IntElement m_RadarDistanceWME;
		private IntElement m_RadarSettingWME;

		private Identifier m_RadarWME;
				
		public Radar() {
			m_RadarStatusWME = m_Agent.CreateStringWME(m_InputLink, kRadarStatusID, kOff);
			m_RadarDistanceWME = m_Agent.CreateIntWME(m_InputLink, kRadarDistanceID, 1);
			m_RadarSettingWME = m_Agent.CreateIntWME(m_InputLink, kRadarSettingID, 1);
		}
		
		public void radarSwitch(boolean setting) {
			if (setting) {
				if (m_RadarStatusWME.GetValue().equalsIgnoreCase(kOff)) {
					// Radar is off, turn it on
					m_Agent.Update(m_RadarStatusWME, kOn);
					if (m_RadarWME == null) {
						m_RadarWME = m_Agent.CreateIdWME(m_InputLink, kRadarID);
					}
				}
			} else {
				if (m_RadarStatusWME.GetValue().equalsIgnoreCase(kOn)) {
					// Radar is on, turn it off
					m_Agent.Update(m_RadarStatusWME, kOff);
					if (m_RadarWME != null) {
						m_Agent.DestroyWME(m_RadarWME);
						m_RadarWME = null;
					}
					for (int i = 0; i < cellIDs.length; ++i) {
						for (int j = 0; j < cellIDs[i].length; ++j) {							
							cellIDs[i][j] = null;
							tankColors[i][j] = null;
						}
					}
				}
			}
		}
		
		public boolean isOn() {
			return m_RadarStatusWME.GetValue().equalsIgnoreCase(kOn);
		}
		
		public void setRadarPower(int setting) {
			if (setting != m_RadarSettingWME.GetValue()) {
				m_Agent.Update(m_RadarSettingWME, setting);
			}
		}
		
		public int getPowerSetting() {
			return m_RadarSettingWME.GetValue();
		}
		
		public void scan(TankSoarWorld world) {
			MapPoint location = new MapPoint(getLocation().x, getLocation().y);
			
			int powerSetting = m_RadarSettingWME.GetValue();
			int actualDistance = 0;
			for (int i = 0; i <= powerSetting; ++i) {
				actualDistance = i;
				if (scanCells(i, world, location) == true) {
					// Blocked
					break;
				}

				// Update for next distance
				location.x += m_RD.xIncrement;
				location.y += m_RD.yIncrement;
			}
			
			if (m_RadarDistanceWME.GetValue() != actualDistance) {
				m_Agent.Update(m_RadarDistanceWME, actualDistance);
			}
		}
		
		private boolean scanCells(int distance, TankSoarWorld world, MapPoint location) {
			for (int i = 0; i < kRadarWidth; ++i) {
				if (cellIDs[i][distance] != null) {
					m_Agent.DestroyWME(cellIDs[i][distance]);
					cellIDs[i][distance] = null;
				}
				tankColors[i][distance] = null;
			}
			
			for (int i = 0; i < kRadarWidth; ++i) {
				// Scan center then left then right
				int position = 0;
				String positionID = null;
				int relativeDirection = 0;
				
				switch (i) {
				case 0:
					position = kRadarCenter;
					positionID = kCenterID;
					relativeDirection = 0;
					break;
				default:
				case 1:
					position = kRadarLeft;
					positionID = kLeftID;
					relativeDirection = m_RD.left;
					break;
				case 2:
					position = kRadarRight;
					positionID = kRightID;
					relativeDirection = m_RD.right;
					break;
				}
				
				if ((position == kRadarCenter) && (distance == 0)) {
					// skip self
					continue;
				}
	
				TankSoarWorld.TankSoarCell cell = world.getCell(location, relativeDirection);
				Tank tank = cell.getTank();
				String id = getCellID(cell);
				
				cellIDs[position][distance] = m_Agent.CreateIdWME(m_RadarWME, id);
				
				m_Agent.CreateIntWME(cellIDs[position][distance], kDistanceID, distance);
				m_Agent.CreateStringWME(cellIDs[position][distance], kPositionID, positionID);
				
				if (id.equalsIgnoreCase(kTankID)) {
					tank.setRWaves(m_RD.backward);
					tankColors[position][distance] = m_Agent.CreateStringWME(cellIDs[position][distance], kColorID, tank.getColor());
				}		

				if ((position == kRadarCenter) && cell.isWall()) {
					return true;
				}
			}
			return false;
		}
		
		public String getRadarID(int x, int y) {
			if (cellIDs[x][y] == null) {
				return null;
			}
			return cellIDs[x][y].GetAttribute();
		}
	}
	
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
	
	private Identifier m_InputLink;
	private StringElement m_BlockedBackwardWME;
	private StringElement m_BlockedForwardWME;
	private StringElement m_BlockedLeftWME;
	private StringElement m_BlockedRightWME;
	private IntElement m_ClockWME;
	private StringElement m_DirectionWME;
	private IntElement m_EnergyWME;
	private StringElement m_EnergyRechargerWME;
	private IntElement m_HealthWME;
	private StringElement m_HealthRechargerWME;
	private StringElement m_IncomingBackwardWME;
	private StringElement m_IncomingForwardWME;
	private StringElement m_IncomingLeftWME;
	private StringElement m_IncomingRightWME;
	private IntElement m_MissilesWME;
	private FloatElement m_RandomWME;
	private StringElement m_ResurrectWME;
	private StringElement m_RWavesBackwardWME;
	private StringElement m_RWavesForwardWME;
	private StringElement m_RWavesLeftWME;
	private StringElement m_RWavesRightWME;
	private StringElement m_ShieldStatusWME;
	private Identifier m_SmellWME;
	private StringElement m_SmellColorWME;
	private IntElement m_SmellDistanceWME;
	private StringElement m_SmellDistanceStringWME;
	private StringElement m_SoundWME;
	private IntElement m_xWME;
	private IntElement m_yWME;
	
	private final static int kInitialEnergy = 1000;
	private final static int kInitialHealth = 1000;
	private final static int kInitialMissiles = 15;

	static private Random random = new Random();
	private Radar m_Radar;
	
	private RelativeDirections m_RD;
	static private int worldCount = 0;
	
	private String m_InitialFacing;
	private MapPoint m_InitialLocation;

	// Call reset() to initialize these:
	private MoveInfo m_LastMove;
	private int m_Resurrect = -1;
	private int m_LastIncoming;
	private int m_RWaves;
	private int m_LastSound;
	private boolean m_Hit;
	
	public Tank(Agent agent, String productions, String color, MapPoint location, String facing, TankSoarWorld world) {
		super(agent, productions, color, location);
		
		if (facing == null) {			
			facing = WorldEntity.kNorth;
		}
		m_InitialFacing = facing;
		m_Facing = facing;
		setFacingInt();
		
		m_InitialLocation = location;
		
		m_RD = new RelativeDirections();	
		m_RD.calculate(getFacingInt());
	
		m_LastMove = new MoveInfo();
		
		m_InputLink = m_Agent.GetInputLink();
		
		Identifier blocked = m_Agent.CreateIdWME(m_InputLink, kBlockedID);
		m_BlockedBackwardWME = m_Agent.CreateStringWME(blocked, kBackwardID, kNo);
		m_BlockedForwardWME = m_Agent.CreateStringWME(blocked, kForwardID, kNo);
		m_BlockedLeftWME = m_Agent.CreateStringWME(blocked, kLeftID, kNo);
		m_BlockedRightWME = m_Agent.CreateStringWME(blocked, kRightID, kNo);
		
		m_ClockWME = m_Agent.CreateIntWME(m_InputLink, kClockID, worldCount);
		
		m_DirectionWME = m_Agent.CreateStringWME(m_InputLink, kDirectionID, getFacing()); 
		
		m_EnergyWME = m_Agent.CreateIntWME(m_InputLink, kEnergyID, kInitialEnergy);
		m_EnergyRechargerWME = m_Agent.CreateStringWME(m_InputLink, kEnergyRechargerID, kNo);
		m_HealthWME = m_Agent.CreateIntWME(m_InputLink, kHealthID, kInitialHealth);
		m_HealthRechargerWME = m_Agent.CreateStringWME(m_InputLink, kHealthRechargerID, kNo);
		
		Identifier incoming = m_Agent.CreateIdWME(m_InputLink, kIncomingID);
		m_IncomingBackwardWME = m_Agent.CreateStringWME(incoming, kBackwardID, kNo);
		m_IncomingForwardWME = m_Agent.CreateStringWME(incoming, kForwardID, kNo);
		m_IncomingLeftWME = m_Agent.CreateStringWME(incoming, kLeftID, kNo);
		m_IncomingRightWME = m_Agent.CreateStringWME(incoming, kRightID, kNo);
		
		m_MissilesWME = m_Agent.CreateIntWME(m_InputLink, kMissilesID, kInitialMissiles);
		m_Agent.CreateStringWME(m_InputLink, kMyColorID, getColor());
		
		// Radar structure depends on situation
		m_Radar = new Radar();
		
		m_RandomWME = m_Agent.CreateFloatWME(m_InputLink, kRandomID, 0);
		m_ResurrectWME = m_Agent.CreateStringWME(m_InputLink, kResurrectID, kYes);
		
		Identifier rwaves = m_Agent.CreateIdWME(m_InputLink, kRWavesID);
		m_RWavesBackwardWME = m_Agent.CreateStringWME(rwaves, kBackwardID, kNo);
		m_RWavesForwardWME = m_Agent.CreateStringWME(rwaves, kForwardID, kNo);
		m_RWavesLeftWME = m_Agent.CreateStringWME(rwaves, kLeftID, kNo);
		m_RWavesRightWME = m_Agent.CreateStringWME(rwaves, kRightID, kNo);
		
		m_ShieldStatusWME = m_Agent.CreateStringWME(m_InputLink, kShieldStatusID, kOff);
		
		m_SmellWME = m_Agent.CreateIdWME(m_InputLink, kSmellID);
		m_SmellColorWME = m_Agent.CreateStringWME(m_SmellWME, kColorID, kNone);
		m_SmellDistanceWME = null;
		m_SmellDistanceStringWME = m_Agent.CreateStringWME(m_SmellWME, kDistanceID, kNone);

		m_SoundWME = m_Agent.CreateStringWME(m_InputLink, kSoundID, kSilentID);
		m_xWME = m_Agent.CreateIntWME(m_InputLink, kXID, getLocation().x);
		m_yWME = m_Agent.CreateIntWME(m_InputLink, kYID, getLocation().y);
		m_Agent.Commit();
						
		reset(world);
	}
	
	public MapPoint getInitialLocation() {
		return m_InitialLocation;
	}
	
	public void reset(TankSoarWorld world) {
		
		// Restore initial values to state
		m_Agent.Update(m_MissilesWME, kInitialMissiles);
		m_Agent.Update(m_EnergyWME, kInitialEnergy);
		m_Agent.Update(m_HealthWME, kInitialHealth);
		m_Agent.Update(m_ResurrectWME, kYes);		
		m_Agent.Update(m_ShieldStatusWME, kOff);
		m_Agent.Update(m_SoundWME, kSilentID);
			
		m_Radar.setRadarPower(1);
		m_Radar.radarSwitch(false);

		m_LastMove.reset();	
		m_LastMove.move = true;	// force blocked, facing, recharger, x, y update
		
		m_Facing = m_InitialFacing;
		setFacingInt();
		
		m_LastIncoming = -1;	// force incoming update
		m_RWaves = 0;		// force rwaves update
		m_LastSound = -1;		// force sound update
		
		m_Hit = false;
		
		// force smell update
		if (m_SmellDistanceWME != null) {
			m_Agent.DestroyWME(m_SmellDistanceWME);
			m_SmellDistanceWME = null;
		}
		if (m_SmellDistanceStringWME != null) {
			m_Agent.DestroyWME(m_SmellDistanceStringWME);
			m_SmellDistanceStringWME = null;
		}
				
		m_Agent.Commit();
		update(world);		// update the rest of input
	}
	
	public Integer getMove() {
		m_LastMove.reset();
		
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
				} 
				
			} else if (commandName.equalsIgnoreCase(kFireID)) {
		 		if (m_MissilesWME.GetValue() > 0) {
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
				
			} else {
				m_Logger.log(getName() + ": Unknown command: " + commandName);
				continue;
			}
			commandId.AddStatusComplete();
		}
		return m_LastMove.move ? new Integer(m_LastMove.moveDirection) : null;
	}
	
	public void update(TankSoarWorld world) {		
		// fire input updated at read time.
		
		TankSoarWorld.TankSoarCell cell = world.getCell(getLocation());
		
		// Movement
		if (m_LastMove.move) {
			// If we moved, we can't rotate
			m_LastMove.rotate = false;
			
			m_Agent.Update(m_EnergyRechargerWME, cell.isEnergyRecharger() ? kYes : kNo);
			m_Agent.Update(m_HealthRechargerWME, cell.isHealthRecharger() ? kYes : kNo);
			
			m_Agent.Update(m_xWME, getLocation().x);
			m_Agent.Update(m_yWME, getLocation().y);
		}
		
		// Rotation
		if (m_LastMove.rotate) {
			rotate(m_LastMove.rotateDirection);
			m_Agent.Update(m_DirectionWME, getFacing());
		}
		
		// Missile damage
		boolean currentShieldStatus = m_ShieldStatusWME.GetValue().equalsIgnoreCase(kOn);
		if (m_Hit) {
			int currentHealth = m_HealthWME.GetValue();
			int currentEnergy = m_EnergyWME.GetValue();
			if (currentShieldStatus) {
				currentEnergy -= kMissileEnergyDamage;
				if (currentEnergy < 0) {
					currentEnergy = 0;
				}
				m_Agent.Update(m_EnergyWME, currentEnergy);
			} else {
				currentHealth -= kMissileHealthDamage;
				if (currentHealth < 0) {
					currentHealth = 0;
				}
				m_Agent.Update(m_HealthWME, currentHealth);
			}
		}
		
		// Chargers
		if (cell.isEnergyRecharger()) {
			// Check for missile hit
			if (m_Hit) {
				// Instant death
				m_Agent.Update(m_HealthWME, 0);
			} else {
				int newEnergy = m_EnergyWME.GetValue() + 250;
				newEnergy = newEnergy > kInitialEnergy ? kInitialEnergy : newEnergy;
				m_Agent.Update(m_EnergyWME, newEnergy);
			}
		}
		if (cell.isHealthRecharger()) {
			// Check for missile hit
			if (m_Hit) {
				// Instant death
				m_Agent.Update(m_HealthWME, 0);
			} else {
				int newHealth = m_HealthWME.GetValue() + 250;
				newHealth = newHealth > kInitialHealth ? kInitialHealth : newHealth;
				m_Agent.Update(m_HealthWME, newHealth);
			}
		}
		
		// Handle shields.
		currentShieldStatus = m_ShieldStatusWME.GetValue().equalsIgnoreCase(kOn);
		boolean desiredShieldStatus = m_LastMove.shields && m_LastMove.shieldsSetting;
		boolean enoughPowerForShields = m_EnergyWME.GetValue() >= kSheildEnergyUsage;
		// If the agent wants to turn the shield on
		if (!currentShieldStatus && desiredShieldStatus) {
			// Turn the shield on if there is enough power.
			if (enoughPowerForShields) {
				currentShieldStatus = true;
				m_Agent.Update(m_ShieldStatusWME, kOn);
			}
		}
		// If the agent wants the shield off or there isn't enough power
		if ((currentShieldStatus && !desiredShieldStatus) || !enoughPowerForShields) {
			// Turn the shield off.
			currentShieldStatus = false;
			m_Agent.Update(m_ShieldStatusWME, kOff);
		}
		// Consume shield energy.
		if (currentShieldStatus) {
			m_Agent.Update(m_EnergyWME, m_EnergyWME.GetValue() - kSheildEnergyUsage);
		}
	
		// Handle radar.
		// Figure out radar power usage.  This is bound by the amount of available energy or the maximum of 14.
		int radarPowerSetting = (m_EnergyWME.GetValue() >= Tank.kRadarHeight) ? Tank.kRadarHeight : m_EnergyWME.GetValue();
		// Lower this number if the user wants to
		if (m_LastMove.radarPower && (m_LastMove.radarPowerSetting < radarPowerSetting)) {
			radarPowerSetting = m_LastMove.radarPowerSetting;
		} else {
			// Or, lower this number using the old radar setting:
			if (m_Radar.getPowerSetting() < radarPowerSetting) {
				radarPowerSetting = m_Radar.getPowerSetting();
			}
		}
		// If the power setting changed, set it.
		if (m_Radar.getPowerSetting() != radarPowerSetting) {
			// Although, turn 0 into 1 first because power setting 0 isn't legal.
			m_Radar.setRadarPower(radarPowerSetting == 0 ? 1 : radarPowerSetting);
		}
		boolean currentRadarStatus = m_Radar.isOn();
		boolean desiredRadarStatus = m_LastMove.radar && m_LastMove.radarSwitch;
		// If the agent wants to turn the radar on
		if (!currentRadarStatus && desiredRadarStatus) {
			// Turn the radar on and scan if there is power.
			if (radarPowerSetting > 0) {
				currentRadarStatus = true;
				m_Radar.radarSwitch(true);
				m_Radar.scan(world);
			}
		}
		// If the agent wants to turn the radar off or there isn't enough power
		if ((currentRadarStatus && !desiredRadarStatus) || (radarPowerSetting == 0)) {
			// Turn the radar off.
			currentRadarStatus = false;
			m_Radar.radarSwitch(false);
		}
		// Consume radar energy.
		if (currentRadarStatus) {
			m_Agent.Update(m_EnergyWME, m_EnergyWME.GetValue() - radarPowerSetting);
		}
		
		// Missiles
		if (m_LastMove.fire) {
			m_Agent.Update(m_MissilesWME, m_MissilesWME.GetValue() - 1);
		}
		
		// Resurrect
		if (m_ResurrectWME.GetValue().equalsIgnoreCase(kYes)) {
			if (m_Resurrect <= 0) {
				m_Resurrect = worldCount;
			} else {
				m_Agent.Update(m_ResurrectWME, kNo);
				m_Resurrect = -1;
			}
		}
		
		m_Agent.Update(m_ClockWME, worldCount);

		// Blocked
		if (m_LastMove.move || m_LastMove.rotate) {		
			int blocked = world.getBlockedByLocation(getLocation());
			m_Agent.Update(m_BlockedForwardWME, ((blocked & m_RD.forward) > 0) ? kYes : kNo);
			m_Agent.Update(m_BlockedBackwardWME, ((blocked & m_RD.backward) > 0) ? kYes : kNo);
			m_Agent.Update(m_BlockedLeftWME, ((blocked & m_RD.left) > 0) ? kYes : kNo);
			m_Agent.Update(m_BlockedRightWME, ((blocked & m_RD.right) > 0) ? kYes : kNo);
		}

		// Incoming
		int incoming = world.getIncomingByLocation(getLocation());
		if (incoming != m_LastIncoming) {
			m_LastIncoming = incoming;
			m_Agent.Update(m_IncomingForwardWME, ((incoming & m_RD.forward) > 0) ? kYes : kNo);
			m_Agent.Update(m_IncomingBackwardWME, ((incoming & m_RD.backward) > 0) ? kYes : kNo);
			m_Agent.Update(m_IncomingLeftWME, ((incoming & m_RD.left) > 0) ? kYes : kNo);
			m_Agent.Update(m_IncomingRightWME, ((incoming & m_RD.right) > 0) ? kYes : kNo);
		}
		
		// Random
		m_Agent.Update(m_RandomWME, random.nextFloat());
				
		// RWaves
		if ((m_RWaves & m_RD.forward) > 0) {
			if (m_RWavesForwardWME.GetValue().equalsIgnoreCase(kNo)) {
				m_Agent.Update(m_RWavesForwardWME, kYes);
			}
		} else {
			if (m_RWavesForwardWME.GetValue().equalsIgnoreCase(kYes)) {
				m_Agent.Update(m_RWavesForwardWME, kNo);
			}
		}
		if ((m_RWaves & m_RD.backward) > 0) {
			if (m_RWavesBackwardWME.GetValue().equalsIgnoreCase(kNo)) {
				m_Agent.Update(m_RWavesBackwardWME, kYes);
			}
		} else {
			if (m_RWavesBackwardWME.GetValue().equalsIgnoreCase(kYes)) {
				m_Agent.Update(m_RWavesBackwardWME, kNo);
			}
		}
		if ((m_RWaves & m_RD.left) > 0) {
			if (m_RWavesLeftWME.GetValue().equalsIgnoreCase(kNo)) {
				m_Agent.Update(m_RWavesLeftWME, kYes);
			}
		} else {
			if (m_RWavesLeftWME.GetValue().equalsIgnoreCase(kYes)) {
				m_Agent.Update(m_RWavesLeftWME, kNo);
			}
		}
		if ((m_RWaves & m_RD.right) > 0) {
			if (m_RWavesRightWME.GetValue().equalsIgnoreCase(kNo)) {
				m_Agent.Update(m_RWavesRightWME, kYes);
			}
		} else {
			if (m_RWavesRightWME.GetValue().equalsIgnoreCase(kYes)) {
				m_Agent.Update(m_RWavesRightWME, kNo);
			}
		}
		
		// Stinky tanks
		Tank closestTank = world.getStinkyTankNear(this);
		if (closestTank != null) {
			// TODO: should check color, distance and not blink if they don't change
			if (m_SmellDistanceWME == null) {
				m_SmellDistanceWME = m_Agent.CreateIntWME(m_SmellWME, kDistanceID, world.getManhattanDistance(this, closestTank));
				if (m_SmellDistanceStringWME != null) {
					m_Agent.DestroyWME(m_SmellDistanceStringWME);
					m_SmellDistanceStringWME = null;
				}
			} else {
				m_Agent.Update(m_SmellDistanceWME, world.getManhattanDistance(this, closestTank));
			}
			m_Agent.Update(m_SmellColorWME, closestTank.getColor());
		} else {
			if (m_SmellDistanceStringWME == null) {
				m_SmellDistanceStringWME = m_Agent.CreateStringWME(m_SmellWME, kDistanceID, kNone);
				m_Agent.Update(m_SmellColorWME, kNone);
				if (m_SmellDistanceWME != null) {
					m_Agent.DestroyWME(m_SmellDistanceWME);
					m_SmellDistanceWME = null;
				}
			}
		}
	
		// Sound
		int sound = 0;//world.getLoudTank(this);
		if (sound != m_LastSound) {
			m_LastSound = sound;
			if (sound == m_RD.forward) {
				m_Agent.Update(m_SoundWME, kForwardID);
			} else if (sound == m_RD.backward) {
				m_Agent.Update(m_SoundWME, kBackwardID);
			} else if (sound == m_RD.left) {
				m_Agent.Update(m_SoundWME, kLeftID);
			} else if (sound == m_RD.right) {
				m_Agent.Update(m_SoundWME, kRightID);
			} else {
				if (sound > 0) {
					m_Logger.log("Warning: sound reported as more than one direction.");
				}
				m_Agent.Update(m_SoundWME, kSilentID);
			}
		}
		
		m_Agent.Commit();
	}
	
	public void addMissiles(int numberToAdd) {
		m_Agent.Update(m_MissilesWME, m_MissilesWME.GetValue() + numberToAdd);
	}
	
	public int getMissiles() {
		return m_MissilesWME.GetValue();
	}
	
	public int getHealth() {
		return m_HealthWME.GetValue();
	}
	
	public void hit() {
		m_Hit = true;
	}
	
	public int getEnergy() {
		return m_EnergyWME.GetValue();
	}
	
	public boolean recentlyMoved() {
		return m_LastMove.move;
	}
	
	static public void setWorldCount(int worldCount) {
		Tank.worldCount = worldCount;
	}
	
	private String getCellID(TankSoarWorld.TankSoarCell cell) {
		if (cell.isWall()) {
			return kObstacleID;
		}
		if (cell.isEnergyRecharger()) {
			return kEnergyID;
		}
		if (cell.isHealthRecharger()) {
			return kHealthID;
		}
		if (cell.containsMissilePack()) {
			return kMissilesID;
		}
		if (cell.containsTank()) {
			return kTankID;
		}
		return kOpenID;
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
	
	public Radar getRadar() {
		return m_Radar;
	}
	
	public boolean firedMissile() {
		return m_LastMove.fire;
	}
	
	public void clearRWaves() {
		m_RWaves = 0;
	}
	
	void setRWaves(int fromDirection) {
		m_RWaves |= fromDirection;
	}
}
