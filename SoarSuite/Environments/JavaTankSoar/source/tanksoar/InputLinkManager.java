package tanksoar;

import java.util.Random;

import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import utilities.Logger;
import utilities.MapPoint;

public class InputLinkManager {

	// public for visual agent world
	public final static String kEnergyID = "energy";
	public final static String kHealthID = "health";
	public final static String kMissilesID = "missiles";
	public final static String kObstacleID = "obstacle";
	public final static String kOpenID = "open";
	public final static String kTankID = "tank";
	
	private final static String kBlockedID = "blocked";
	private final static String kClockID = "clock";
	private final static String kColorID = "color";
	private final static String kDirectionID = "direction";
	private final static String kDistanceID = "distance";
	private final static String kEnergyRechargerID = "energyrecharger";
	private final static String kHealthRechargerID = "healthrecharger";
	private final static String kIncomingID = "incoming";
	private final static String kMyColorID = "my-color";
	private final static String kRadarID = "radar";
	private final static String kRadarDistanceID = "radar-distance";
	private final static String kRadarSettingID = "radar-setting";
	private final static String kRadarStatusID = "radar-status";
	private final static String kRandomID = "random";
	private final static String kResurrectID = "resurrect";
	private final static String kRWavesID = "rwaves";
	private final static String kShieldStatusID = "shield-status";
	private final static String kSmellID = "smell";
	private final static String kSoundID = "sound";
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

	private static final int kRadarLeft = 0;
	private static final int kRadarCenter = 1;
	private static final int kRadarRight = 2;
	
	private Random m_Random = new Random();

	private Identifier m_InputLink;
	private Identifier m_BlockedWME;
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
	private Identifier m_IncomingWME;
	private StringElement m_IncomingBackwardWME;
	private StringElement m_IncomingForwardWME;
	private StringElement m_IncomingLeftWME;
	private StringElement m_IncomingRightWME;
	private IntElement m_MissilesWME;
	private StringElement m_RadarStatusWME;
	private IntElement m_RadarDistanceWME;
	private IntElement m_RadarSettingWME;
	private Identifier m_RadarWME;
	private FloatElement m_RandomWME;
	private StringElement m_ResurrectWME;
	private Identifier m_RWavesWME;
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

	private Logger m_Logger = Logger.logger;
	
	private Identifier[][] radarCellIDs = new Identifier[Tank.kRadarWidth][Tank.kRadarHeight];
	private StringElement[][] radarColors = new StringElement[Tank.kRadarWidth][Tank.kRadarHeight];
	
	private Agent m_Agent;
	private Tank m_Tank;
	private TankSoarWorld m_World;
	private boolean m_Reset = true;
	private int m_ResurrectFrame = 0;

	public InputLinkManager(TankSoarWorld world, Tank tank) {
		m_Agent = tank.getAgent();
		m_World = world;
		m_Tank = tank;
		m_InputLink = m_Agent.GetInputLink();
	}
	
	public void clear() {
		if (m_Reset == true) {
			return;
		}
		
		m_Agent.DestroyWME(m_BlockedWME);
		m_Agent.DestroyWME(m_ClockWME);
		m_Agent.DestroyWME(m_DirectionWME);
		m_Agent.DestroyWME(m_EnergyWME);
		m_Agent.DestroyWME(m_EnergyRechargerWME);
		m_Agent.DestroyWME(m_HealthWME);
		m_Agent.DestroyWME(m_HealthRechargerWME);
		m_Agent.DestroyWME(m_IncomingWME);
		m_Agent.DestroyWME(m_MissilesWME);
		m_Agent.DestroyWME(m_RadarStatusWME);
		m_Agent.DestroyWME(m_RadarDistanceWME);
		m_Agent.DestroyWME(m_RadarSettingWME);
		m_Agent.DestroyWME(m_RadarWME);
		m_Agent.DestroyWME(m_RandomWME);
		m_Agent.DestroyWME(m_ResurrectWME);
		m_Agent.DestroyWME(m_RWavesWME);
		m_Agent.DestroyWME(m_ShieldStatusWME);
		m_Agent.DestroyWME(m_SmellWME);
		m_Agent.DestroyWME(m_SoundWME);
		m_Agent.DestroyWME(m_xWME);
		m_Agent.DestroyWME(m_yWME);
		m_Agent.Commit();

		clearRadar();

		m_Reset = true;
	}
	
	void update() {
		MapPoint location = m_Tank.getLocation();
		TankSoarCell cell = m_World.getCell(location);
		
		String energyRecharger = cell.isEnergyRecharger() ? kYes : kNo;
		String healthRecharger = cell.isHealthRecharger() ? kYes : kNo;

		if (m_Reset) {
			m_EnergyRechargerWME = m_Agent.CreateStringWME(m_InputLink, kEnergyRechargerID, energyRecharger);
			m_HealthRechargerWME = m_Agent.CreateStringWME(m_InputLink, kHealthRechargerID, healthRecharger);

			m_xWME = m_Agent.CreateIntWME(m_InputLink, kXID, location.x);
			m_yWME = m_Agent.CreateIntWME(m_InputLink, kYID, location.y);
			
		} else {
			if (m_Tank.recentlyMoved()) {
				m_Agent.Update(m_EnergyRechargerWME, energyRecharger);
				m_Agent.Update(m_HealthRechargerWME, healthRecharger);
				
				m_Agent.Update(m_xWME, location.x);
				m_Agent.Update(m_yWME, location.y);
			}
		}

		int currentEnergy = m_Tank.getEnergy();
		int currentHealth = m_Tank.getHealth();
		if (m_Reset) {
			m_EnergyWME = m_Agent.CreateIntWME(m_InputLink, kEnergyID, currentEnergy);
			m_HealthWME = m_Agent.CreateIntWME(m_InputLink, kHealthID, currentHealth);
		} else {
			if (m_EnergyWME.GetValue() != currentEnergy) {
				m_Agent.Update(m_EnergyWME, currentEnergy);
			}
			if (m_HealthWME.GetValue() != currentHealth) {
				m_Agent.Update(m_HealthWME, currentHealth);
			}			
		}

		String shieldStatus = m_Tank.getShieldStatus() ? kOn : kOff;
		if (m_Reset) {
			m_ShieldStatusWME = m_Agent.CreateStringWME(m_InputLink, kShieldStatusID, shieldStatus);
		} else {
			if (!m_ShieldStatusWME.GetValue().equalsIgnoreCase(shieldStatus)) {
				m_Agent.Update(m_ShieldStatusWME, shieldStatus);
			}
		}
		
		String facing = m_Tank.getFacing();
		if (m_Reset) {
			m_DirectionWME = m_Agent.CreateStringWME(m_InputLink, kDirectionID, facing);
		} else {
			if (!m_DirectionWME.GetValue().equalsIgnoreCase(facing)) {
				m_Agent.Update(m_DirectionWME, facing);
			}
		}
				
		int blocked = m_World.getBlockedByLocation(location);
		
		String blockedForward = ((blocked & m_Tank.forward()) > 0) ? kYes : kNo;
		String blockedBackward = ((blocked & m_Tank.backward()) > 0) ? kYes : kNo;
		String blockedLeft = ((blocked & m_Tank.left()) > 0) ? kYes : kNo;
		String blockedRight = ((blocked & m_Tank.right()) > 0) ? kYes : kNo;
		
		if (m_Reset) {
			m_BlockedWME = m_Agent.CreateIdWME(m_InputLink, kBlockedID);
			m_BlockedForwardWME = m_Agent.CreateStringWME(m_BlockedWME, kForwardID, blockedForward);
			m_BlockedBackwardWME = m_Agent.CreateStringWME(m_BlockedWME, kBackwardID, blockedBackward);
			m_BlockedLeftWME = m_Agent.CreateStringWME(m_BlockedWME, kLeftID, blockedLeft);
			m_BlockedRightWME = m_Agent.CreateStringWME(m_BlockedWME, kRightID, blockedRight);				
		} else {
			if (m_Tank.recentlyMoved() || !m_BlockedForwardWME.GetValue().equalsIgnoreCase(blockedForward)) {
				m_Agent.Update(m_BlockedForwardWME, blockedForward);
			}
			if (m_Tank.recentlyMoved() || !m_BlockedBackwardWME.GetValue().equalsIgnoreCase(blockedBackward)) {
				m_Agent.Update(m_BlockedBackwardWME, blockedBackward);
			}
			if (m_Tank.recentlyMoved() || !m_BlockedLeftWME.GetValue().equalsIgnoreCase(blockedLeft)) {
				m_Agent.Update(m_BlockedLeftWME, blockedLeft);
			}
			if (m_Tank.recentlyMoved() || !m_BlockedRightWME.GetValue().equalsIgnoreCase(blockedRight)) {
				m_Agent.Update(m_BlockedRightWME, blockedRight);
			}
		}
		
		int incoming = m_World.getIncomingByLocation(location);
		String incomingForward = ((incoming & m_Tank.forward()) > 0) ? kYes : kNo;
		String incomingBackward = ((incoming & m_Tank.backward()) > 0) ? kYes : kNo;
		String incomingLeft = ((incoming & m_Tank.left()) > 0) ? kYes : kNo;
		String incomingRight = ((incoming & m_Tank.right()) > 0) ? kYes : kNo;
		
		if (m_Reset) {
			m_IncomingWME = m_Agent.CreateIdWME(m_InputLink, kIncomingID);
			m_IncomingBackwardWME = m_Agent.CreateStringWME(m_IncomingWME, kBackwardID, incomingForward);
			m_IncomingForwardWME = m_Agent.CreateStringWME(m_IncomingWME, kForwardID, incomingBackward);
			m_IncomingLeftWME = m_Agent.CreateStringWME(m_IncomingWME, kLeftID, incomingLeft);
			m_IncomingRightWME = m_Agent.CreateStringWME(m_IncomingWME, kRightID, incomingRight);
			
		} else {
			if (!m_IncomingForwardWME.GetValue().equalsIgnoreCase(incomingForward)) {
				m_Agent.Update(m_IncomingForwardWME, incomingForward);
			}
			if (!m_IncomingBackwardWME.GetValue().equalsIgnoreCase(incomingBackward)) {
				m_Agent.Update(m_IncomingBackwardWME, incomingBackward);
			}
			if (!m_IncomingLeftWME.GetValue().equalsIgnoreCase(incomingLeft)) {
				m_Agent.Update(m_IncomingLeftWME, incomingLeft);
			}
			if (!m_IncomingRightWME.GetValue().equalsIgnoreCase(incomingRight)) {
				m_Agent.Update(m_IncomingRightWME, incomingRight);
			}
		}

		// Sound
		int sound = m_World.getSoundNear(m_Tank);
		String soundString;
		if (sound == m_Tank.forward()) {
			soundString = kForwardID;
		} else if (sound == m_Tank.backward()) {
			soundString = kBackwardID;
		} else if (sound == m_Tank.left()) {
			soundString = kLeftID;
		} else if (sound == m_Tank.right()) {
			soundString = kRightID;
		} else {
			if (sound > 0) {
				m_Logger.log("Warning: sound reported as more than one direction.");
			}
			soundString = kSilentID;
		}
		if (m_Reset) {
			m_SoundWME = m_Agent.CreateStringWME(m_InputLink, kSoundID, soundString);			
		} else {
			if (!m_SoundWME.GetValue().equalsIgnoreCase(soundString)) {
				m_Agent.Update(m_SoundWME, soundString);
			}
		}
		
		// Smell
		Tank closestTank = m_World.getStinkyTankNear(m_Tank);
		String closestTankColor = (closestTank == null) ? kNone : closestTank.getColor();
		int distance = (closestTank == null) ? 0 : location.getManhattanDistanceTo(closestTank.getLocation());
		if (m_Reset) {
			m_SmellWME = m_Agent.CreateIdWME(m_InputLink, kSmellID);
			m_SmellColorWME = m_Agent.CreateStringWME(m_SmellWME, kColorID, closestTankColor);
			if (closestTank == null) {
				m_SmellDistanceWME = null;
				m_SmellDistanceStringWME = m_Agent.CreateStringWME(m_SmellWME, kDistanceID, kNone);
			} else {
				m_SmellDistanceWME = m_Agent.CreateIntWME(m_SmellWME, kDistanceID, distance);
				m_SmellDistanceStringWME = null;
			}
		} else {
			if (!m_SmellColorWME.GetValue().equalsIgnoreCase(closestTankColor)) {
				m_Agent.Update(m_SmellColorWME, closestTankColor);
			}
			if (closestTank == null) {
				if (m_SmellDistanceWME != null) {
					m_Agent.DestroyWME(m_SmellDistanceWME);
					m_SmellDistanceWME = null;
				}
				if (m_SmellDistanceStringWME == null) {
					m_SmellDistanceStringWME = m_Agent.CreateStringWME(m_SmellWME, kDistanceID, kNone);
				}
			} else {
				if (m_SmellDistanceWME == null) {
					m_SmellDistanceWME = m_Agent.CreateIntWME(m_SmellWME, kDistanceID, distance);
				} else {
					if (m_SmellDistanceWME.GetValue() != distance) {
						m_Agent.Update(m_SmellDistanceWME, distance);
					}
				}
				if (m_SmellDistanceStringWME != null) {
					m_Agent.DestroyWME(m_SmellDistanceStringWME);
					m_SmellDistanceStringWME = null;
				}
			}
		}

		// Missiles
		int missiles = m_Tank.getMissiles();
		if (m_Reset) {
			m_MissilesWME = m_Agent.CreateIntWME(m_InputLink, kMissilesID, missiles);
		} else {
			if (m_MissilesWME.GetValue() != missiles) {
				m_Agent.Update(m_MissilesWME, missiles);
			}
		}
		
		// Color
		if (m_Reset) {
			m_Agent.CreateStringWME(m_InputLink, kMyColorID, m_Tank.getColor());
		}
		
		int worldCount = m_World.getWorldCount();
		if (m_Reset) {
			m_ClockWME = m_Agent.CreateIntWME(m_InputLink, kClockID, worldCount);
		} else {
			m_Agent.Update(m_ClockWME, worldCount);
		}
		
		// Resurrect
		if (m_Reset) {
			m_ResurrectFrame = worldCount;
			m_ResurrectWME = m_Agent.CreateStringWME(m_InputLink, kResurrectID, kYes);
		} else {
			if (worldCount != m_ResurrectFrame) {
				if (!m_ResurrectWME.GetValue().equalsIgnoreCase(kNo)) {
					m_Agent.Update(m_ResurrectWME, kNo);
				}
			}
		}
		
		// Radar
		String radarStatus = m_Tank.getRadarStatus() ? kOn : kOff;
		if (m_Reset) {
			m_RadarStatusWME = m_Agent.CreateStringWME(m_InputLink, kRadarStatusID, radarStatus);
			if (m_Tank.getRadarStatus()) {
				m_RadarWME = m_Agent.CreateIdWME(m_InputLink, kRadarID);
			} else {
				m_RadarWME = null;
			}
		} else {
			if (!m_RadarStatusWME.GetValue().equalsIgnoreCase(radarStatus)) {
				m_Agent.Update(m_RadarStatusWME, radarStatus);
			}
			if (m_Tank.getRadarStatus()) {
				if (m_RadarWME == null) {
					m_RadarWME = m_Agent.CreateIdWME(m_InputLink, kRadarID);
				}
			} else {
				if (m_RadarWME != null) {
					m_Agent.DestroyWME(m_RadarWME);
					m_RadarWME = null;
					clearRadar();
				}
			}
		}
		
		int radarSetting = m_Tank.getRadarSetting();
		int radarDistance = m_Tank.getRadarStatus() ? scan(radarSetting) : 0;
		m_Tank.setRadarDistance(radarDistance);
		
		if (m_Reset) {
			m_RadarDistanceWME = m_Agent.CreateIntWME(m_InputLink, kRadarDistanceID, radarDistance);
			m_RadarSettingWME = m_Agent.CreateIntWME(m_InputLink, kRadarSettingID, radarSetting);
		} else {
			if (m_RadarDistanceWME.GetValue() != radarDistance) {
				m_Agent.Update(m_RadarDistanceWME, radarDistance);
			}
			if (m_RadarSettingWME.GetValue() != radarSetting) {
				m_Agent.Update(m_RadarSettingWME, radarSetting);
			}
		}
		
		// Random
		float random = m_Random.nextFloat();
		if (m_Reset) {
			m_RandomWME = m_Agent.CreateFloatWME(m_InputLink, kRandomID, random);
		} else {
			m_Agent.Update(m_RandomWME, random);
		}
		
	}
	
	void updateRWaves() {
		// RWaves
		int rwaves = m_Tank.getRWaves();
		String rwavesForward = (rwaves & m_Tank.forward()) > 0 ? kYes : kNo;
		String rwavesBackward = (rwaves & m_Tank.backward()) > 0 ? kYes : kNo;;
		String rwavesLeft = (rwaves & m_Tank.left()) > 0 ? kYes : kNo;
		String rwavesRight = (rwaves & m_Tank.right()) > 0 ? kYes : kNo;
		
		if (m_Reset) {
			m_RWavesWME = m_Agent.CreateIdWME(m_InputLink, kRWavesID);
			m_RWavesForwardWME = m_Agent.CreateStringWME(m_RWavesWME, kForwardID, rwavesBackward);
			m_RWavesBackwardWME = m_Agent.CreateStringWME(m_RWavesWME, kBackwardID, rwavesForward);
			m_RWavesLeftWME = m_Agent.CreateStringWME(m_RWavesWME, kLeftID, rwavesLeft);
			m_RWavesRightWME = m_Agent.CreateStringWME(m_RWavesWME, kRightID, rwavesRight);
		} else {
			if (!m_RWavesForwardWME.GetValue().equalsIgnoreCase(rwavesForward)) {
				m_Agent.Update(m_RWavesForwardWME, rwavesForward);
			}
			if (!m_RWavesBackwardWME.GetValue().equalsIgnoreCase(rwavesBackward)) {
				m_Agent.Update(m_RWavesBackwardWME, rwavesBackward);
			}
			if (!m_RWavesLeftWME.GetValue().equalsIgnoreCase(rwavesLeft)) {
				m_Agent.Update(m_RWavesLeftWME, rwavesLeft);
			}
			if (!m_RWavesRightWME.GetValue().equalsIgnoreCase(rwavesRight)) {
				m_Agent.Update(m_RWavesRightWME, rwavesRight);
			}
			
		}	
		
		m_Reset = false;
		m_Agent.Commit();
	}

	private void clearRadar() {
		for (int i = 0; i < Tank.kRadarWidth; ++i) {
			for (int j = 0; j < Tank.kRadarHeight; ++j) {
				radarCellIDs[i][j] = null;
				radarColors[i][j] = null;
			}
		}
	}
	
	private int scan(int setting) {
		MapPoint location = new MapPoint(m_Tank.getLocation());
		
		int distance = 0;
		for (int i = 0; i <= setting; ++i) {
			distance = i;
			if (scanCells(i, location) == true) {
				// Blocked
				clearCells(i, location);
				break;
			}

			// Update for next distance
			location.travel(m_Tank.forward());
		}
		return distance;
	}
	
	private boolean scanCells(int distance, MapPoint location) {
		for (int i = 0; i < Tank.kRadarWidth; ++i) {
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
				relativeDirection = m_Tank.left();
				break;
			case 2:
				position = kRadarRight;
				positionID = kRightID;
				relativeDirection = m_Tank.right();
				break;
			}
			
			if ((position == kRadarCenter) && (distance == 0)) {
				// skip self
				continue;
			}

			TankSoarCell cell = m_World.getCell(location, relativeDirection);
			Tank tank = cell.getTank();
			String id = getCellID(cell);
			boolean tankID = id.equalsIgnoreCase(kTankID);
			if (tankID && tank == null) {
				m_Logger.log("This should never happen.");
			}

			if (radarCellIDs[position][distance] == null) {
				radarCellIDs[position][distance] = m_Agent.CreateIdWME(m_RadarWME, id);
				m_Agent.CreateIntWME(radarCellIDs[position][distance], kDistanceID, distance);
				m_Agent.CreateStringWME(radarCellIDs[position][distance], kPositionID, positionID);
				if (tankID) {
					tank.setRWaves(m_Tank.backward());
					radarColors[position][distance] = m_Agent.CreateStringWME(radarCellIDs[position][distance], kColorID, tank.getColor());
				}
			} else {
				if (m_Tank.recentlyMoved() || cell.isModified()) {
					m_Agent.DestroyWME(radarCellIDs[position][distance]);
					radarCellIDs[position][distance] = m_Agent.CreateIdWME(m_RadarWME, id);
					m_Agent.CreateIntWME(radarCellIDs[position][distance], kDistanceID, distance);
					m_Agent.CreateStringWME(radarCellIDs[position][distance], kPositionID, positionID);
					if (tankID) {
						tank.setRWaves(m_Tank.backward());
						radarColors[position][distance] = m_Agent.CreateStringWME(radarCellIDs[position][distance], kColorID, tank.getColor());
					}		
				}
			}
			
			if ((position == kRadarCenter) && cell.isBlocked()) {
				return true;
			}
			cell.setRadarTouch();
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
	
				if (radarCellIDs[i][j] == null) {
					continue;
				} 
				m_Agent.DestroyWME(radarCellIDs[i][j]);
				radarCellIDs[i][j] = null;
				radarColors[i][j] = null;
			}
		}
	}
	
	private String getCellID(TankSoarCell cell) {
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
	
	public String getRadarID(int x, int y) {
		if (radarCellIDs[x][y] == null) {
			return null;
		}
		return radarCellIDs[x][y].GetAttribute();
	}
}

















