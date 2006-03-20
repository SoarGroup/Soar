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

	public final static String kEast = "east";
	public final static String kNorth = "north";
	public final static String kSouth = "south";
	public final static String kWest = "west";
	
	private final static String kYes = "yes";
	private final static String kNo = "no";
	
	private final static String kOff = "off";
	private final static String kOn = "on";
	
	private final static String kNone = "none";
	
	private final static String kCenter = "center";
	
	
	public class MoveInfo {
		boolean move;
		String moveDirection;
		
		boolean fire;
		
		public MoveInfo() {
			reset();
		}
		
		public void reset() {
			move = fire = false;
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
	private StringElement m_MyColorWME;
	private Identifier m_RadarWME;
	private IntElement m_RadarDistanceWME;
	private IntElement m_RadarSettingWME;
	private StringElement m_RadarStatusWME;
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

	// Call reset() to initialize these:
	private int m_Missiles;
	private int m_Health;
	private int m_Energy;
	private int m_RadarDistance;
	private int m_RadarSetting;
	private boolean m_RadarStatus;
	private boolean m_Resurrect;
	private boolean m_Shields;
	private MoveInfo m_LastMove;
	static private int worldCount;
	
	public Tank(Agent agent, String productions, String color, Point location) {
		super(agent, productions, color, location);
		reset();
		
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
		m_MyColorWME = m_Agent.CreateStringWME(m_InputLink, kMyColorID, color);
		
		// Radar structure depends on situation
		
		m_RadarDistanceWME = m_Agent.CreateIntWME(m_InputLink, kRadarDistanceID, m_RadarDistance);
		m_RadarSettingWME = m_Agent.CreateIntWME(m_InputLink, kRadarSettingID, m_RadarSetting);
		m_RadarStatusWME = m_Agent.CreateStringWME(m_InputLink, kRadarStatusID, kOff);
		m_RandomWME = m_Agent.CreateFloatWME(m_InputLink, kRandomID, 0);
		m_ResurrectWME = m_Agent.CreateStringWME(m_InputLink, kResurrectID, kNo);
		
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
		
		int blocked = world.getBlockedByLocation(getLocation());
		m_Agent.Update(m_BlockedForwardWME, ((blocked & rd.forward) > 0) ? kYes : kNo);
		m_Agent.Update(m_BlockedBackwardWME, ((blocked & rd.backward) > 0) ? kYes : kNo);
		m_Agent.Update(m_BlockedLeftWME, ((blocked & rd.left) > 0) ? kYes : kNo);
		m_Agent.Update(m_BlockedRightWME, ((blocked & rd.right) > 0) ? kYes : kNo);
		
		m_Agent.Update(m_ClockWME, worldCount);
		
		m_Agent.Update(m_DirectionWME, getFacing());
		
		m_Agent.Update(m_EnergyWME, m_Energy);
		m_Agent.Update(m_EnergyRechargerWME, cell.isEnergyRecharger() ? kYes : kNo);
		
		m_Agent.Update(m_HealthWME, m_Health);
		m_Agent.Update(m_HealthRechargerWME, cell.isHealthRecharger() ? kYes : kNo);
		
		int incoming = world.getIncomingByLocation(getLocation());
		m_Agent.Update(m_IncomingForwardWME, ((incoming & rd.forward) > 0) ? kYes : kNo);
		m_Agent.Update(m_IncomingBackwardWME, ((incoming & rd.backward) > 0) ? kYes : kNo);
		m_Agent.Update(m_IncomingLeftWME, ((incoming & rd.left) > 0) ? kYes : kNo);
		m_Agent.Update(m_IncomingRightWME, ((incoming & rd.right) > 0) ? kYes : kNo);
		
		m_Agent.Update(m_MissilesWME, m_Missiles);

		m_Agent.Update(m_MyColorWME, getColor());		
		
		if (m_RadarStatus) {
			if (m_RadarWME == null) {
				m_RadarWME = m_Agent.CreateIdWME(m_InputLink, kRadarID);
			}
			Point location = new Point(getLocation().x, getLocation().y);
			
			String cellID;
			Identifier cellWME;
			for (int i = 0; i <= m_RadarSetting; ++i) {
				// TODO: these three should be in a loop but I am lazy right now.
				
				// Center
				if (i > 0) {
					cellID = getCellID(world.getCell(location));
					cellWME = m_Agent.CreateIdWME(m_RadarWME, cellID);
	
					m_Agent.CreateIntWME(cellWME, kDistanceID, i);
					m_Agent.CreateStringWME(cellWME, kPositionID, kCenter);
					
					if (cellID == kTankID) {
						m_Agent.CreateStringWME(cellWME, kColorID, world.getCell(location).getTank().getColor());
					}
				}
				
				// Left
				cellID = getCellID(world.getCell(location, rd.left));
				cellWME = m_Agent.CreateIdWME(m_RadarWME, cellID);

				m_Agent.CreateIntWME(cellWME, kDistanceID, i);
				m_Agent.CreateStringWME(cellWME, kPositionID, kLeftID);
				
				if (cellID == kTankID) {
					m_Agent.CreateStringWME(cellWME, kColorID, world.getCell(location, rd.left).getTank().getColor());
				}

				// Right
				cellID = getCellID(world.getCell(location, rd.right));
				cellWME = m_Agent.CreateIdWME(m_RadarWME, cellID);

				m_Agent.CreateIntWME(cellWME, kDistanceID, i);
				m_Agent.CreateStringWME(cellWME, kPositionID, kRightID);
				
				if (cellID == kTankID) {
					m_Agent.CreateStringWME(cellWME, kColorID, world.getCell(location, rd.right).getTank().getColor());
				}

				// Update for next distance
				location.x += rd.xIncrement;
				location.y += rd.yIncrement;
			}
		} else {
			if (m_RadarWME != null) {
				m_Agent.DestroyWME(m_RadarWME);
				m_RadarWME = null;
			}
		}
		
		m_Agent.Update(m_RadarDistanceWME, m_RadarDistance);
		m_Agent.Update(m_RadarSettingWME, m_RadarSetting);
		m_Agent.Update(m_RadarStatusWME, m_RadarStatus ? kOn : kOff);

		m_Agent.Update(m_RandomWME, random.nextFloat());
		
		m_Agent.Update(m_ResurrectWME, m_Resurrect ? kYes : kNo);
		
		int rwaves = world.getRWavesByLocation(getLocation());
		m_Agent.Update(m_RWavesForwardWME, ((rwaves & rd.forward) > 0) ? kYes : kNo);
		m_Agent.Update(m_RWavesBackwardWME, ((rwaves & rd.backward) > 0) ? kYes : kNo);
		m_Agent.Update(m_RWavesLeftWME, ((rwaves & rd.left) > 0) ? kYes : kNo);
		m_Agent.Update(m_RWavesRightWME, ((rwaves & rd.right) > 0) ? kYes : kNo);

		m_Agent.Update(m_ShieldStatusWME, m_Shields ? kOn : kOff);
		
		Tank closestTank = world.getStinkyTankNearLocation(getLocation());
		if (closestTank != null) {
			if (m_SmellDistanceWME != null) {
				m_Agent.Update(m_SmellColorWME, closestTank.getColor());
				m_Agent.Update(m_SmellDistanceWME, getManhattanDistanceTo(closestTank));
			} else {
				m_SmellDistanceWME = m_Agent.CreateIntWME(m_SmellWME, kDistanceID, 0);			
			}
			if (m_SmellDistanceStringWME != null) {
				m_Agent.DestroyWME(m_SmellDistanceStringWME);
				m_SmellDistanceStringWME = null;
			}
		} else {
			if (m_SmellDistanceStringWME != null) {
				m_Agent.Update(m_SmellColorWME, kNone);
				m_Agent.Update(m_SmellDistanceStringWME, kNone);
			} else {
				m_SmellDistanceStringWME = m_Agent.CreateStringWME(m_SmellWME, kDistanceID, kNone);
			}
			if (m_SmellDistanceWME != null) {
				m_Agent.DestroyWME(m_SmellDistanceWME);
				m_SmellDistanceWME = null;
			}
		}
	
		int sound = world.getSoundByLocation(getLocation());
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
		
		m_Agent.Update(m_xWME, getLocation().x);
		m_Agent.Update(m_yWME, getLocation().y);

		m_Agent.Commit();
	}
	
	public MoveInfo getMove() {
		m_LastMove.reset();
		
		if (m_Agent.GetNumberCommands() == 0) {
			m_Logger.log(getName() + " issued no command.");
			return null;
		}
		
		boolean moved = false;
		boolean rotated = false;

		RelativeDirections rd = new RelativeDirections(getFacingInt());		
		
		for (int i = 0; i < m_Agent.GetNumberCommands(); ++i) {
		
			Identifier commandId = m_Agent.GetCommand(i);
			String commandName = commandId.GetAttribute();

			// TODO: make sure commands can happen simultaneously
			if (commandName.equalsIgnoreCase(kMoveID)) {
				if (rotated) {
					m_Logger.log("Tried to move and rotate at the same time.");
				}
				moved = true;
				
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
				m_RadarStatus = commandId.GetParameterValue(kSwitchID).equalsIgnoreCase(kOn) ? true : false;
			} else if (commandName.equalsIgnoreCase(kRadarPowerID)) {
				//m_RadarSetting = Integer.decode(commandId.GetParameterValue(kRadarPowerID)).intValue();
			} else if (commandName.equalsIgnoreCase(kShieldsID)) {
				m_Shields = commandId.GetParameterValue(kSwitchID).equalsIgnoreCase(kOn) ? true : false;
			} else if (commandName.equalsIgnoreCase(kRotateID)) {
				if (moved) {
					m_Logger.log("Tried to move and rotate at the same time.");
				}
				rotated = true;
				rotate(commandId.GetParameterValue(kDirectionID));
			} else {
				m_Logger.log("Unknown command: " + commandName);
				continue;
			}
			commandId.AddStatusComplete();
		}

		return m_LastMove;
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
		
		// TODO: initial direction setting? using north
		m_Facing = WorldEntity.kNorth;
		setFacingInt();
		
		m_RadarDistance = 0;
		m_RadarSetting = 0;
		m_RadarStatus = false;
		m_Resurrect = false;
		m_Shields = false;
		m_LastMove = new MoveInfo();
		worldCount = 0;
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
