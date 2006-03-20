package tanksoar;

import java.util.*;

import org.eclipse.swt.graphics.*;

import simulation.*;
import sml.*;
import utilities.*;

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
	private final static String kSettingID = "setting";
	private final static String kShields = "shields";
	private final static String kShieldStatusID = "shield-status";
	private final static String kSmellID = "smell";
	private final static String kSoundID = "sound";
	private final static String kSwitchID = "switch";
	private final static String kWeaponID = "missile";
	private final static String kXID = "x";
	private final static String kYID = "y";
	
	private final static String kBackwardID = "backward";
	private final static String kForwardID = "forward";
	private final static String kLeftID = "left";
	private final static String kRightID = "right";
	private final static String kSilentID = "silent";
	private final static String kMissileID = "missile";
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
	private final static String kLeft = "left";
	private final static String kRight = "right";
	
	
	public class MoveInfo {
		boolean move;
		String moveDirection;
		
		boolean rotate;
		String rotateDirection;
		
		boolean fire;
		
		boolean radar;
		boolean radarSetting;
		
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
	
	private MoveInfo m_LastMove = new MoveInfo();
	
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
	private StringElement m_SmellColorWME;
	private IntElement m_SmellDistanceWME;
	private StringElement m_SoundWME;
	private IntElement m_xWME;
	private IntElement m_yWME;
	
	private final static int kInitialEnergy = 1000;
	private final static int kInitialHealth = 1000;
	private final static int kInitialMissiles = 15;
	
	private int m_Missiles = kInitialMissiles;
	private int m_Health = kInitialHealth;
	private int m_Energy = kInitialEnergy;
	private int m_RadarDistance = 0;
	private int m_RadarSetting = 0;
	private boolean m_RadarStatus = false;
	private boolean m_Resurrect = false;
	private boolean m_Shields = false;
	
	static private int worldCount = 0;
	static private Random random = new Random();
	
	public Tank(Agent agent, String productions, String color, Point location) {
		super(agent, productions, color, location);
		
		Identifier inputLink = m_Agent.GetInputLink();
		
		Identifier blocked = m_Agent.CreateIdWME(inputLink, kBlockedID);
		m_BlockedBackwardWME = m_Agent.CreateStringWME(blocked, kBackwardID, kNo);
		m_BlockedForwardWME = m_Agent.CreateStringWME(blocked, kForwardID, kNo);
		m_BlockedLeftWME = m_Agent.CreateStringWME(blocked, kLeftID, kNo);
		m_BlockedRightWME = m_Agent.CreateStringWME(blocked, kRightID, kNo);
		
		m_ClockWME = m_Agent.CreateIntWME(inputLink, kClockID, worldCount);
		
		// TODO: initial direction setting?
		m_DirectionWME = m_Agent.CreateStringWME(inputLink, kDirectionID, kNorth); 
		m_EnergyWME = m_Agent.CreateIntWME(inputLink, kEnergyID, kInitialEnergy);
		m_EnergyRechargerWME = m_Agent.CreateStringWME(inputLink, kEnergyRechargerID, kNo);
		m_HealthWME = m_Agent.CreateIntWME(inputLink, kHealthID, kInitialHealth);
		m_HealthRechargerWME = m_Agent.CreateStringWME(inputLink, kHealthRechargerID, kNo);
		
		Identifier incoming = m_Agent.CreateIdWME(inputLink, kIncomingID);
		m_IncomingBackwardWME = m_Agent.CreateStringWME(incoming, kBackwardID, kNo);
		m_IncomingForwardWME = m_Agent.CreateStringWME(incoming, kForwardID, kNo);
		m_IncomingLeftWME = m_Agent.CreateStringWME(incoming, kLeftID, kNo);
		m_IncomingRightWME = m_Agent.CreateStringWME(incoming, kRightID, kNo);
		
		m_MissilesWME = m_Agent.CreateIntWME(inputLink, kMissilesID, kInitialMissiles);
		m_MyColorWME = m_Agent.CreateStringWME(inputLink, kMyColorID, color);
		
		m_RadarWME = m_Agent.CreateIdWME(inputLink, kRadarID);
		// Radar substructure depends on situation
		
		m_RadarDistanceWME = m_Agent.CreateIntWME(inputLink, kRadarDistanceID, m_RadarDistance);
		m_RadarSettingWME = m_Agent.CreateIntWME(inputLink, kRadarSettingID, m_RadarSetting);
		m_RadarStatusWME = m_Agent.CreateStringWME(inputLink, kRadarStatusID, kOff);
		m_RandomWME = m_Agent.CreateFloatWME(inputLink, kRandomID, 0);
		m_ResurrectWME = m_Agent.CreateStringWME(inputLink, kResurrectID, kNo);
		
		Identifier rwaves = m_Agent.CreateIdWME(inputLink, kRWavesID);
		m_RWavesBackwardWME = m_Agent.CreateStringWME(rwaves, kBackwardID, kNo);
		m_RWavesForwardWME = m_Agent.CreateStringWME(rwaves, kForwardID, kNo);
		m_RWavesLeftWME = m_Agent.CreateStringWME(rwaves, kLeftID, kNo);
		m_RWavesRightWME = m_Agent.CreateStringWME(rwaves, kRightID, kNo);
		
		m_ShieldStatusWME = m_Agent.CreateStringWME(inputLink, kShieldStatusID, kOff);
		
		Identifier smell = m_Agent.CreateIdWME(inputLink, kSmellID);
		m_SmellColorWME = m_Agent.CreateStringWME(smell, kColorID, kNone);
		m_SmellDistanceWME = m_Agent.CreateIntWME(smell, kDistanceID, 0);

		m_SoundWME = m_Agent.CreateStringWME(inputLink, kSoundID, kSilentID);
		m_xWME = m_Agent.CreateIntWME(inputLink, kXID, location.x);
		m_yWME = m_Agent.CreateIntWME(inputLink, kYID, location.y);
						
		m_Agent.Commit();		
	}
	
	public void updateInput(TankSoarWorld world) {
		// assign directions depending on facing
		int forward = 0, backward = 0, left = 0, right = 0, xIncrement = 0, yIncrement = 0;
		
		switch (getFacingInt()) {
			case WorldEntity.kNorthInt:
				forward = kNorthInt;
				backward = kSouthInt;
				left = kWestInt;
				right = kEastInt;
				yIncrement = -1;
				break;
			case WorldEntity.kEastInt:
				forward = kEastInt;
				backward = kWestInt;
				left = kNorthInt;
				right = kSouthInt;
				xIncrement = 1;
				break;
			case WorldEntity.kSouthInt:
				forward = kSouthInt;
				backward = kNorthInt;
				left = kEastInt;
				right = kWestInt;
				yIncrement = 1;
				break;
			case WorldEntity.kWestInt:
				forward = kWestInt;
				backward = kEastInt;
				left = kSouthInt;
				right = kNorthInt;
				xIncrement = -1;
				break;
		}
		
		// Get current cell
		TankSoarWorld.TankSoarCell cell = world.getCell(getLocation());
		
		int blocked = world.getBlockedByLocation(getLocation());
		m_Agent.Update(m_BlockedForwardWME, ((blocked & forward) > 0) ? kYes : kNo);
		m_Agent.Update(m_BlockedBackwardWME, ((blocked & backward) > 0) ? kYes : kNo);
		m_Agent.Update(m_BlockedLeftWME, ((blocked & left) > 0) ? kYes : kNo);
		m_Agent.Update(m_BlockedRightWME, ((blocked & right) > 0) ? kYes : kNo);
		
		m_Agent.Update(m_ClockWME, worldCount);
		
		m_Agent.Update(m_DirectionWME, getFacing());
		
		m_Agent.Update(m_EnergyWME, m_Energy);
		m_Agent.Update(m_EnergyRechargerWME, cell.isEnergyRecharger() ? kYes : kNo);
		
		m_Agent.Update(m_HealthWME, m_Health);
		m_Agent.Update(m_HealthRechargerWME, cell.isHealthRecharger() ? kYes : kNo);
		
		int incoming = world.getIncomingByLocation(getLocation());
		m_Agent.Update(m_IncomingForwardWME, ((incoming & forward) > 0) ? kYes : kNo);
		m_Agent.Update(m_IncomingBackwardWME, ((incoming & backward) > 0) ? kYes : kNo);
		m_Agent.Update(m_IncomingLeftWME, ((incoming & left) > 0) ? kYes : kNo);
		m_Agent.Update(m_IncomingRightWME, ((incoming & right) > 0) ? kYes : kNo);
		
		m_Agent.Update(m_MissilesWME, m_Missiles);

		m_Agent.Update(m_MyColorWME, getColor());		
		
		if (m_RadarStatus) {
			Point location = getLocation();
			String cellID;
			Identifier cellWME;
			for (int i = 0; i <= m_RadarSetting; ++i) {
				// TODO: these three should be in a loop but I am lazy right now.
				
				// Center
				cellID = getCellID(world.getCell(location));
				cellWME = m_Agent.CreateIdWME(m_RadarWME, cellID);

				m_Agent.CreateIntWME(cellWME, kDistanceID, i);
				m_Agent.CreateStringWME(cellWME, kPositionID, kCenter);
				
				if (cellID == kTankID) {
					m_Agent.CreateStringWME(cellWME, kColorID, world.getCell(location).getTank().getColor());
				}
				
				// Left
				cellID = getCellID(world.getCell(location, left));
				cellWME = m_Agent.CreateIdWME(m_RadarWME, cellID);

				m_Agent.CreateIntWME(cellWME, kDistanceID, i);
				m_Agent.CreateStringWME(cellWME, kPositionID, kLeft);
				
				if (cellID == kTankID) {
					m_Agent.CreateStringWME(cellWME, kColorID, world.getCell(location, left).getTank().getColor());
				}

				// Right
				cellID = getCellID(world.getCell(location, right));
				cellWME = m_Agent.CreateIdWME(m_RadarWME, cellID);

				m_Agent.CreateIntWME(cellWME, kDistanceID, i);
				m_Agent.CreateStringWME(cellWME, kPositionID, kRight);
				
				if (cellID == kTankID) {
					m_Agent.CreateStringWME(cellWME, kColorID, world.getCell(location, right).getTank().getColor());
				}

				// Update for next distance
				location.x += xIncrement;
				location.y += yIncrement;
			}
		}
		
		m_Agent.Update(m_RadarDistanceWME, m_RadarDistance);
		m_Agent.Update(m_RadarSettingWME, m_RadarSetting);
		m_Agent.Update(m_RadarStatusWME, m_RadarStatus ? kOn : kOff);

		m_Agent.Update(m_RandomWME, random.nextFloat());
		
		m_Agent.Update(m_ResurrectWME, m_Resurrect ? kYes : kNo);
		
		int rwaves = world.getRWavesByLocation(getLocation());
		m_Agent.Update(m_RWavesForwardWME, ((rwaves & forward) > 0) ? kYes : kNo);
		m_Agent.Update(m_RWavesBackwardWME, ((rwaves & backward) > 0) ? kYes : kNo);
		m_Agent.Update(m_RWavesLeftWME, ((rwaves & left) > 0) ? kYes : kNo);
		m_Agent.Update(m_RWavesRightWME, ((rwaves & right) > 0) ? kYes : kNo);

		m_Agent.Update(m_ShieldStatusWME, m_Shields ? kOn : kOff);
		
		Tank closestTank = world.getStinkyTankNearLocation(getLocation());
		if (closestTank != null) {
			m_Agent.Update(m_SmellColorWME, closestTank.getColor());
			m_Agent.Update(m_SmellDistanceWME, getManhattanDistanceTo(closestTank));
		} else {
			m_Agent.Update(m_SmellColorWME, kNone);
			m_Agent.Update(m_SmellDistanceWME, 0);
		}
	
		int sound = world.getSoundByLocation(getLocation());
		if (sound == forward) {
			m_Agent.Update(m_SoundWME, kForwardID);
		} else if (sound == backward) {
			m_Agent.Update(m_SoundWME, kBackwardID);
		} else if (sound == left) {
			m_Agent.Update(m_SoundWME, kLeftID);
		} else if (sound == right) {
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
	
	private void createRadarWME(TankSoarWorld world, Point location, int distance, String directionString) {
	}
	
	public MoveInfo getMove() {
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
	
	public void resetMissiles() {
		m_Missiles = kInitialMissiles;
	}
	
	public void resetHealth() {
		m_Health = kInitialHealth;
	}
	
	public void resetEnergy() {
		m_Energy = kInitialEnergy;
	}
	
	static public void setWorldCount(int worldCount) {
		Tank.worldCount = worldCount;
	}
	
	private int getManhattanDistanceTo(Tank tank) {
		// TODO:
		return 0;
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
		if (cell.isMissiles()) {
			return kMissilesID;
		}
		if (cell.isTank()) {
			return kTankID;
		}
		return kOpenID;
	}
}
