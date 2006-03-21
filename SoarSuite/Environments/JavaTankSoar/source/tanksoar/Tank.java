package tanksoar;

import java.util.*;

import org.eclipse.swt.graphics.*;

import simulation.*;
import sml.*;

public class Tank  extends WorldEntity {
	
	private final static String kBlockedID = "blocked";
	private final static String kClockID = "clock";
	private final static String kColorID = "color";
	private final static String kDirectionID = "direction";
	private final static String kDistanceID = "distance";
	private final static String kEnergyID = "energy";
	private final static String kEnergyRechargerID = "energyrecharger";
	private final static String kFireID = "fire";
	private final static String kHealthID = "health";
	private final static String kHealthRechargerID = "healthrecharger";
	private final static String kIncomingID = "incoming";
	private final static String kMissilesID = "missiles";
	private final static String kMoveID = "move";
	private final static String kMyColorID = "my-color";
	private final static String kObstacleID = "obstacle";
	private final static String kOpenID = "open";
	private final static String kTankID = "tank";
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
	
	
	public class Radar {
		//0[left  0][left  1][left  2]
		//1[tank  0][center1][center2]-->facing
		//2[right 0][right 1][right 2]
		private static final int kRadarLeft = 0;
		private static final int kRadarCenter = 1;
		private static final int kRadarRight = 2;
		
		private Identifier[][] cellIDs;
		private StringElement[][] tankColors;
		
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
				// Turn radar on if off
				if (m_RadarStatusWME.GetValue().equalsIgnoreCase(kOff)) {
					// Radar is off
					m_Agent.Update(m_RadarStatusWME, kOn);
					cellIDs = new Identifier[3][14];
					tankColors = new StringElement[3][14];
				}
			} else {
				// Turn radar off if on
				if (m_RadarStatusWME.GetValue().equalsIgnoreCase(kOn)) {
					// Radar is on
					m_Agent.Update(m_RadarStatusWME, kOff);
					for (int i = 0; i < cellIDs.length; ++i) {
						for (int j = 0; j < cellIDs[i].length; ++j) {
							if (cellIDs[i][j] != null) {
								m_Agent.DestroyWME(cellIDs[i][j]);
								cellIDs[i][j] = null;
							}
							if (tankColors[i][j] != null) {
								m_Agent.DestroyWME(tankColors[i][j]);
								tankColors[i][j] = null;
							}
						}
					}
				}
			}
		}
		
		public void setRadarPower(int setting) {
			if (setting != m_RadarSettingWME.GetValue()) {
				m_Agent.Update(m_RadarSettingWME, setting);
			}
		}
		
		public void scan(TankSoarWorld world, RelativeDirections rd) {
			if (m_RadarStatusWME.GetValue().equalsIgnoreCase(kOff)) {
				// Radar is off
				if (m_RadarWME != null) {
					m_Agent.DestroyWME(m_RadarWME);
					m_RadarWME = null;
				}
				return;
			}
			
			// Force an update if we moved or rotated
			boolean forceUpdate = m_LastMove.move || m_LastMove.rotate;
			
			// Radar is on
			if (m_RadarWME == null) {
				// Force an update if we just turned on the radar
				forceUpdate = true;
				m_RadarWME = m_Agent.CreateIdWME(m_InputLink, kRadarID);
			}
			
			Point location = new Point(getLocation().x, getLocation().y);
			
			int powerSetting = m_RadarSettingWME.GetValue();
			int actualDistance = 0;
			for (int i = 0; i <= powerSetting; ++i) {
				actualDistance = i;
				if (scanCells(forceUpdate, i, world, location, rd) == true) {
					// Blocked
					break;
				}

				// Update for next distance
				location.x += rd.xIncrement;
				location.y += rd.yIncrement;
			}
			
			if (m_RadarDistanceWME.GetValue() != actualDistance) {
				m_Agent.Update(m_RadarDistanceWME, actualDistance);
			}
		}
		
		private boolean scanCells(boolean update, int distance, TankSoarWorld world, Point location, RelativeDirections rd) {
			boolean blocked = false;
			
			for (int position = 0; position < 3; ++position) {
				
				if ((position == kRadarCenter) && (distance == 0)) {
					// skip self
					continue;
				}
				
				String positionID = null;
				int relativeDirection = 0;
				
				switch (position) {
				case kRadarLeft:
					positionID = kLeftID;
					relativeDirection = rd.left;
					break;
				default:
				case kRadarCenter:
					positionID = kCenterID;
					relativeDirection = 0;
					break;
				case kRadarRight:
					positionID = kRightID;
					relativeDirection = rd.right;
					break;
				}
	
				String id = getCellID(world.getCell(location, relativeDirection));
				
				String tankColor = null;
				
				// if the update isn't forced
				if (!update) {
					// do we have an old id?
					String oldId = null;
					if (cellIDs[position][distance] == null) {
						update = true;
					} else {
						// flag an update if the ids don't match
						oldId = cellIDs[position][distance].GetIdentifierName();
						if (!id.equalsIgnoreCase(oldId)) {
							update = true;
						} else {
							// The IDs do match but if it is a tank id...
							if (id.equalsIgnoreCase(kTankID)) {
								
								// it may be a different tank, compare colors
								tankColor = world.getCell(location, rd.left).getTank().getColor();
								if (!tankColor.equalsIgnoreCase(tankColors[position][distance].GetValue())) {
									// colors are different!
									update = true;
								}
							}
						}
					}
				}
				
				if (update) {
					if (cellIDs[position][distance] != null) {
						m_Agent.DestroyWME(cellIDs[position][distance]);
						tankColors[position][distance] = null;
					}
					cellIDs[position][distance] = m_Agent.CreateIdWME(m_RadarWME, id);
					
					m_Agent.CreateIntWME(cellIDs[position][distance], kDistanceID, distance);
					m_Agent.CreateStringWME(cellIDs[position][distance], kPositionID, positionID);
					
					if (id.equalsIgnoreCase(kTankID)) {
						tankColor = world.getCell(location, rd.left).getTank().getColor();
						tankColors[position][distance] = m_Agent.CreateStringWME(cellIDs[position][distance], kColorID, tankColor);
					}		

					if ((position == kRadarCenter) && world.getCell(location, relativeDirection).isWall()) {
						blocked = true;
					}
				}
			}
			
			return blocked;
		}
	}
	
	public class MoveInfo {
		boolean move;
		String moveDirection;
		
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

	// Call reset() to initialize these:
	private int m_Missiles;
	private boolean m_ReceivedMissiles;
	private int m_Health;
	private int m_Energy;
	private MoveInfo m_LastMove;
	static private int worldCount = 0;
	private int m_LastIncoming;
	private int m_LastRWaves;
	private int m_LastSound;
	
	public Tank(Agent agent, String productions, String color, Point location) {
		super(agent, productions, color, location);
		
		// TODO: initial direction setting? using north
		// TODO: reset facing in reset() ?
		m_Facing = WorldEntity.kNorth;
		setFacingInt();
		
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
						
		reset();
		
		m_Agent.Commit();		
	}
	
	public class RelativeDirections {
		public int forward;
		public int backward;
		public int left;
		public int right;
		public int xIncrement = 0;
		public int yIncrement = 0;
		public String forwardString;
		public String backwardString;
		public String leftString;
		public String rightString;
		
		public RelativeDirections(int facing) {
				switch (facing) {
				case WorldEntity.kNorthInt:
					forward = kNorthInt;
					backward = kSouthInt;
					left = kWestInt;
					right = kEastInt;
					forwardString = kNorth;
					backwardString = kSouth;
					leftString = kWest;
					rightString = kEast;
					yIncrement = -1;
					break;
				case WorldEntity.kEastInt:
					forward = kEastInt;
					backward = kWestInt;
					left = kNorthInt;
					right = kSouthInt;
					forwardString = kEast;
					backwardString = kWest;
					leftString = kNorth;
					rightString = kSouth;
					xIncrement = 1;
					break;
				case WorldEntity.kSouthInt:
					forward = kSouthInt;
					backward = kNorthInt;
					left = kEastInt;
					right = kWestInt;
					forwardString = kSouth;
					backwardString = kNorth;
					leftString = kEast;
					rightString = kWest;
					yIncrement = 1;
					break;
				case WorldEntity.kWestInt:
					forward = kWestInt;
					backward = kEastInt;
					left = kSouthInt;
					right = kNorthInt;
					forwardString = kWest;
					backwardString = kEast;
					leftString = kSouth;
					rightString = kNorth;
					xIncrement = -1;
					break;
			}
		}
	}
	
	public void updateInput(TankSoarWorld world) {
		RelativeDirections rd = new RelativeDirections(getFacingInt());
		
		// Get current cell
		TankSoarWorld.TankSoarCell cell = world.getCell(getLocation());

		m_Agent.Update(m_ClockWME, worldCount);
		m_Agent.Update(m_EnergyWME, m_Energy);
		m_Agent.Update(m_HealthWME, m_Health);
	
		if (m_LastMove.move || m_LastMove.rotate) {		
			int blocked = world.getBlockedByLocation(getLocation());
			m_Agent.Update(m_BlockedForwardWME, ((blocked & rd.forward) > 0) ? kYes : kNo);
			m_Agent.Update(m_BlockedBackwardWME, ((blocked & rd.backward) > 0) ? kYes : kNo);
			m_Agent.Update(m_BlockedLeftWME, ((blocked & rd.left) > 0) ? kYes : kNo);
			m_Agent.Update(m_BlockedRightWME, ((blocked & rd.right) > 0) ? kYes : kNo);

			m_Agent.Update(m_DirectionWME, getFacing());			

			m_Agent.Update(m_EnergyRechargerWME, cell.isEnergyRecharger() ? kYes : kNo);
			m_Agent.Update(m_HealthRechargerWME, cell.isHealthRecharger() ? kYes : kNo);
		}
		
		int incoming = world.getIncomingByLocation(getLocation());
		if (incoming != m_LastIncoming) {
			m_LastIncoming = incoming;
			m_Agent.Update(m_IncomingForwardWME, ((incoming & rd.forward) > 0) ? kYes : kNo);
			m_Agent.Update(m_IncomingBackwardWME, ((incoming & rd.backward) > 0) ? kYes : kNo);
			m_Agent.Update(m_IncomingLeftWME, ((incoming & rd.left) > 0) ? kYes : kNo);
			m_Agent.Update(m_IncomingRightWME, ((incoming & rd.right) > 0) ? kYes : kNo);
		}
		
		if (m_ReceivedMissiles || m_LastMove.fire) {
			m_ReceivedMissiles = false;
			m_Agent.Update(m_MissilesWME, m_Missiles);
		}

		m_Radar.scan(world, rd);
		
		m_Agent.Update(m_RandomWME, random.nextFloat());
				
		int rwaves = world.getRWavesByLocation(getLocation());
		if (rwaves != m_LastRWaves) {
			m_LastRWaves = rwaves;
			m_Agent.Update(m_RWavesForwardWME, ((rwaves & rd.forward) > 0) ? kYes : kNo);
			m_Agent.Update(m_RWavesBackwardWME, ((rwaves & rd.backward) > 0) ? kYes : kNo);
			m_Agent.Update(m_RWavesLeftWME, ((rwaves & rd.left) > 0) ? kYes : kNo);
			m_Agent.Update(m_RWavesRightWME, ((rwaves & rd.right) > 0) ? kYes : kNo);
		}
		
		Tank closestTank = world.getStinkyTankNearLocation(getLocation());
		if (closestTank != null) {
			if (m_SmellDistanceWME == null) {
				// TODO: should check color, distance and not blink if they don't change
				m_SmellDistanceWME = m_Agent.CreateIntWME(m_SmellWME, kDistanceID, getManhattanDistanceTo(closestTank));			
				m_Agent.Update(m_SmellColorWME, closestTank.getColor());
				m_Agent.DestroyWME(m_SmellDistanceStringWME);
				m_SmellDistanceStringWME = null;
			}
		} else {
			if (m_SmellDistanceStringWME == null) {
				m_SmellDistanceStringWME = m_Agent.CreateStringWME(m_SmellWME, kDistanceID, kNone);
				m_Agent.Update(m_SmellColorWME, kNone);
				m_Agent.DestroyWME(m_SmellDistanceWME);
				m_SmellDistanceWME = null;
			}
		}
	
		int sound = world.getSoundByLocation(getLocation());
		if (sound != m_LastSound) {
			if (sound == rd.forward) {
				m_Agent.Update(m_SoundWME, kForwardID);
			} else if (sound == rd.backward) {
				m_Agent.Update(m_SoundWME, kBackwardID);
			} else if (sound == rd.left) {
				m_Agent.Update(m_SoundWME, kLeftID);
			} else if (sound == rd.right) {
				m_Agent.Update(m_SoundWME, kRightID);
			} else {
				if (sound > 0) {
					m_Logger.log("Warning: sound reported as more than one direction.");
				}
				m_Agent.Update(m_SoundWME, kSilentID);
			}
		}
		
		m_Agent.Update(m_xWME, getLocation().x);
		m_Agent.Update(m_yWME, getLocation().y);

		m_Agent.Commit();
	}
	
	public MoveInfo getMove() {
		m_LastMove.reset();
		
		if (m_ResurrectWME.GetValue().equalsIgnoreCase(kYes)) {
			m_Agent.Update(m_ResurrectWME, kNo);
		}
		
		if (m_Agent.GetNumberCommands() == 0) {
			m_Logger.log(getName() + " issued no command.");
			return null;
		}
		
		RelativeDirections rd = new RelativeDirections(getFacingInt());		
		
		for (int i = 0; i < m_Agent.GetNumberCommands(); ++i) {
		
			Identifier commandId = m_Agent.GetCommand(i);
			String commandName = commandId.GetAttribute();

			// TODO: make sure commands can happen simultaneously
			if (commandName.equalsIgnoreCase(kMoveID)) {
				if (m_LastMove.rotate) {
					m_Logger.log("Tried to move and rotate at the same time.");
				}
				m_LastMove.move = true;
				
				if (commandId.GetParameterValue(kDirectionID).equalsIgnoreCase(kForwardID)) {
					m_LastMove.moveDirection = rd.forwardString;
				} else if (commandId.GetParameterValue(kDirectionID).equalsIgnoreCase(kBackwardID)) {
					m_LastMove.moveDirection = rd.backwardString;
				} else if (commandId.GetParameterValue(kDirectionID).equalsIgnoreCase(kLeftID)) {
					m_LastMove.moveDirection = rd.leftString;
				} else if (commandId.GetParameterValue(kDirectionID).equalsIgnoreCase(kRightID)) {
					m_LastMove.moveDirection = rd.rightString;
				} 
				
				
			} else if (commandName.equalsIgnoreCase(kFireID)) {
				m_LastMove.fire = true;
				// Weapon ignored
				
			} else if (commandName.equalsIgnoreCase(kRadarID)) {
				m_LastMove.radar = true;
				m_LastMove.radarSwitch = commandId.GetParameterValue(kSwitchID).equalsIgnoreCase(kOn) ? true : false;  
				
				m_Radar.radarSwitch(m_LastMove.radarSwitch);
				
			} else if (commandName.equalsIgnoreCase(kRadarPowerID)) {
				
				String powerValue = commandId.GetParameterValue(kSettingID);
				if (powerValue != null) {
					try {
						m_LastMove.radarPowerSetting = Integer.decode(powerValue).intValue();
					} catch (NumberFormatException e) {
						m_Logger.log("Unable to parse radar power setting " + powerValue + ": " + e.getMessage());
					}
					m_Radar.setRadarPower(m_LastMove.radarPowerSetting);
					m_LastMove.radarPower = true;
				} else {
					m_Logger.log("Radar power setting is null.");
				}
				
			} else if (commandName.equalsIgnoreCase(kShieldsID)) {
				m_LastMove.shields = true;
				m_LastMove.shieldsSetting = commandId.GetParameterValue(kSwitchID).equalsIgnoreCase(kOn) ? true : false; 
				
				m_Agent.Update(m_ShieldStatusWME,commandId.GetParameterValue(kSwitchID));
				
			} else if (commandName.equalsIgnoreCase(kRotateID)) {
				if (m_LastMove.move) {
					m_Logger.log("Tried to move and rotate at the same time.");
				}
				m_LastMove.rotate = true;
				m_LastMove.rotateDirection = commandId.GetParameterValue(kDirectionID);
				
				rotate(commandId.GetParameterValue(kDirectionID));
			} else {
				m_Logger.log("Unknown command: " + commandName);
				continue;
			}
			commandId.AddStatusComplete();
		}

		return m_LastMove;
	}
	
	public void addMissiles(int numberToAdd) {
		m_ReceivedMissiles = true;
		m_Missiles += numberToAdd;
	}
	
	public int getMissiles() {
		return m_Missiles;
	}
	
	public int getHealth() {
		return m_Health;
	}
	
	public int getEnergy() {
		return m_Energy;
	}
	
	public void reset() {
		m_Missiles = kInitialMissiles;
		m_Health = kInitialHealth;
		m_Energy = kInitialEnergy;
		
		m_Radar.setRadarPower(1);
		m_Radar.radarSwitch(false);
		m_Agent.Update(m_ResurrectWME, kYes);
		
		m_Agent.Update(m_ShieldStatusWME, kOff);
		m_LastMove = new MoveInfo();
		
		worldCount = 0;
		
		m_LastIncoming = 0;
		m_Agent.Update(m_IncomingForwardWME, kNo);
		m_Agent.Update(m_IncomingBackwardWME, kNo);
		m_Agent.Update(m_IncomingLeftWME, kNo);
		m_Agent.Update(m_IncomingRightWME, kNo);
		
		m_LastRWaves = 0;
		m_Agent.Update(m_RWavesForwardWME, kNo);
		m_Agent.Update(m_RWavesBackwardWME, kNo);
		m_Agent.Update(m_RWavesLeftWME, kNo);
		m_Agent.Update(m_RWavesRightWME, kNo);
		
		m_LastSound = 0;
		m_Agent.Update(m_SoundWME, kSilentID);
		
		m_ReceivedMissiles = false;
		m_Agent.Update(m_MissilesWME, m_Missiles);
		m_Agent.Commit();
	}
	
	static public void setWorldCount(int worldCount) {
		Tank.worldCount = worldCount;
	}
	
	private int getManhattanDistanceTo(Tank tank) {
		Point mine = getLocation();
		Point other = tank.getLocation();
		
		return Math.abs(mine.x - other.x) + Math.abs(mine.y - other.y);
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
		RelativeDirections rd = new RelativeDirections(getFacingInt());
		
		if (direction.equalsIgnoreCase(kLeftID)) {
			m_Facing = rd.leftString;
		} else if (direction.equalsIgnoreCase(kRightID)) {
			m_Facing = rd.rightString;
		}
		setFacingInt();
	}
}
