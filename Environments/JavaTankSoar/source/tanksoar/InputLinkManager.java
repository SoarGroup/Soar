package tanksoar;

import java.util.Random;

import simulation.Simulation;
import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import sml.WMElement;
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
	
	private void DestroyWME(WMElement wme) {
		assert wme != null;
		m_Agent.DestroyWME(wme);
		wme = null;
	}

	private void Update(StringElement wme, String value) {
		assert wme != null;
		assert value != null;
		m_Agent.Update(wme, value);
	}

	private void Update(IntElement wme, int value) {
		assert wme != null;
		m_Agent.Update(wme, value);
	}

	private void Update(FloatElement wme, float value) {
		assert wme != null;
		m_Agent.Update(wme, value);
	}
	
	private IntElement CreateIntWME(Identifier id, String attribute, int value) {
		assert id != null;
		assert attribute != null;
		return m_Agent.CreateIntWME(id, attribute, value);
	}

	private StringElement CreateStringWME(Identifier id, String attribute, String value) {
		assert id != null;
		assert attribute != null;
		assert value != null;
		return m_Agent.CreateStringWME(id, attribute, value);
	}

	private FloatElement CreateFloatWME(Identifier id, String attribute, float value) {
		assert id != null;
		assert attribute != null;
		return m_Agent.CreateFloatWME(id, attribute, value);
	}

	public void clear() {
		if (m_Reset == true) {
			return;
		}
		
		DestroyWME(m_BlockedWME);
		DestroyWME(m_ClockWME);
		DestroyWME(m_DirectionWME);
		DestroyWME(m_EnergyWME);
		DestroyWME(m_EnergyRechargerWME);
		DestroyWME(m_HealthWME);
		DestroyWME(m_HealthRechargerWME);
		DestroyWME(m_IncomingWME);
		DestroyWME(m_MissilesWME);
		DestroyWME(m_RadarStatusWME);
		DestroyWME(m_RadarDistanceWME);
		DestroyWME(m_RadarSettingWME);
		if (m_RadarWME != null) {
			DestroyWME(m_RadarWME);
		}
		DestroyWME(m_RandomWME);
		DestroyWME(m_ResurrectWME);
		DestroyWME(m_RWavesWME);
		DestroyWME(m_ShieldStatusWME);
		DestroyWME(m_SmellWME);
		DestroyWME(m_SoundWME);
		DestroyWME(m_xWME);
		DestroyWME(m_yWME);
		m_Agent.Commit();

		clearRadar();

		m_Reset = true;
	}
	
	void write() {
		MapPoint location = m_Tank.getLocation();
		TankSoarCell cell = m_World.getCell(location);
		
		String energyRecharger = cell.isEnergyRecharger() ? kYes : kNo;
		String healthRecharger = cell.isHealthRecharger() ? kYes : kNo;

		if (m_Reset) {
			m_EnergyRechargerWME = CreateStringWME(m_InputLink, kEnergyRechargerID, energyRecharger);
			m_HealthRechargerWME = CreateStringWME(m_InputLink, kHealthRechargerID, healthRecharger);

			m_xWME = CreateIntWME(m_InputLink, kXID, location.x);
			m_yWME = CreateIntWME(m_InputLink, kYID, location.y);
			
		} else {
			if (m_Tank.recentlyMoved()) {
				Update(m_EnergyRechargerWME, energyRecharger);
				Update(m_HealthRechargerWME, healthRecharger);
				
				Update(m_xWME, location.x);
				Update(m_yWME, location.y);
			}
		}

		int currentEnergy = m_Tank.getEnergy();
		int currentHealth = m_Tank.getHealth();
		if (m_Reset) {
			m_EnergyWME = CreateIntWME(m_InputLink, kEnergyID, currentEnergy);
			m_HealthWME = CreateIntWME(m_InputLink, kHealthID, currentHealth);
		} else {
			if (m_EnergyWME.GetValue() != currentEnergy) {
				Update(m_EnergyWME, currentEnergy);
			}
			if (m_HealthWME.GetValue() != currentHealth) {
				Update(m_HealthWME, currentHealth);
			}			
		}

		String shieldStatus = m_Tank.getShieldStatus() ? kOn : kOff;
		if (m_Reset) {
			m_ShieldStatusWME = CreateStringWME(m_InputLink, kShieldStatusID, shieldStatus);
		} else {
			if (!m_ShieldStatusWME.GetValue().equalsIgnoreCase(shieldStatus)) {
				Update(m_ShieldStatusWME, shieldStatus);
			}
		}
		
		String facing = m_Tank.getFacing();
		if (m_Reset) {
			m_DirectionWME = CreateStringWME(m_InputLink, kDirectionID, facing);
		} else {
			if (!m_DirectionWME.GetValue().equalsIgnoreCase(facing)) {
				Update(m_DirectionWME, facing);
			}
		}
				
		int blocked = m_World.getBlockedByLocation(m_Tank);
		
		String blockedForward = ((blocked & m_Tank.forward()) > 0) ? kYes : kNo;
		String blockedBackward = ((blocked & m_Tank.backward()) > 0) ? kYes : kNo;
		String blockedLeft = ((blocked & m_Tank.left()) > 0) ? kYes : kNo;
		String blockedRight = ((blocked & m_Tank.right()) > 0) ? kYes : kNo;
		
		if (m_Reset) {
			m_BlockedWME = m_Agent.CreateIdWME(m_InputLink, kBlockedID);
			m_BlockedForwardWME = CreateStringWME(m_BlockedWME, kForwardID, blockedForward);
			m_BlockedBackwardWME = CreateStringWME(m_BlockedWME, kBackwardID, blockedBackward);
			m_BlockedLeftWME = CreateStringWME(m_BlockedWME, kLeftID, blockedLeft);
			m_BlockedRightWME = CreateStringWME(m_BlockedWME, kRightID, blockedRight);				
		} else {
			if (m_Tank.recentlyMovedOrRotated() || !m_BlockedForwardWME.GetValue().equalsIgnoreCase(blockedForward)) {
				Update(m_BlockedForwardWME, blockedForward);
			}
			if (m_Tank.recentlyMovedOrRotated() || !m_BlockedBackwardWME.GetValue().equalsIgnoreCase(blockedBackward)) {
				Update(m_BlockedBackwardWME, blockedBackward);
			}
			if (m_Tank.recentlyMovedOrRotated() || !m_BlockedLeftWME.GetValue().equalsIgnoreCase(blockedLeft)) {
				Update(m_BlockedLeftWME, blockedLeft);
			}
			if (m_Tank.recentlyMovedOrRotated() || !m_BlockedRightWME.GetValue().equalsIgnoreCase(blockedRight)) {
				Update(m_BlockedRightWME, blockedRight);
			}
		}
		
		int incoming = m_World.getIncomingByLocation(location);
		String incomingForward = ((incoming & m_Tank.forward()) > 0) ? kYes : kNo;
		String incomingBackward = ((incoming & m_Tank.backward()) > 0) ? kYes : kNo;
		String incomingLeft = ((incoming & m_Tank.left()) > 0) ? kYes : kNo;
		String incomingRight = ((incoming & m_Tank.right()) > 0) ? kYes : kNo;
		
		if (m_Reset) {
			m_IncomingWME = m_Agent.CreateIdWME(m_InputLink, kIncomingID);
			m_IncomingBackwardWME = CreateStringWME(m_IncomingWME, kBackwardID, incomingForward);
			m_IncomingForwardWME = CreateStringWME(m_IncomingWME, kForwardID, incomingBackward);
			m_IncomingLeftWME = CreateStringWME(m_IncomingWME, kLeftID, incomingLeft);
			m_IncomingRightWME = CreateStringWME(m_IncomingWME, kRightID, incomingRight);
			
		} else {
			if (!m_IncomingForwardWME.GetValue().equalsIgnoreCase(incomingForward)) {
				Update(m_IncomingForwardWME, incomingForward);
			}
			if (!m_IncomingBackwardWME.GetValue().equalsIgnoreCase(incomingBackward)) {
				Update(m_IncomingBackwardWME, incomingBackward);
			}
			if (!m_IncomingLeftWME.GetValue().equalsIgnoreCase(incomingLeft)) {
				Update(m_IncomingLeftWME, incomingLeft);
			}
			if (!m_IncomingRightWME.GetValue().equalsIgnoreCase(incomingRight)) {
				Update(m_IncomingRightWME, incomingRight);
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
			m_SoundWME = CreateStringWME(m_InputLink, kSoundID, soundString);			
		} else {
			if (!m_SoundWME.GetValue().equalsIgnoreCase(soundString)) {
				Update(m_SoundWME, soundString);
			}
		}
		
		// Smell
		Tank closestTank = m_World.getStinkyTankNear(m_Tank);
		String closestTankColor = (closestTank == null) ? kNone : closestTank.getColor();
		int distance = (closestTank == null) ? 0 : location.getManhattanDistanceTo(closestTank.getLocation());
		m_Tank.setSmellDistance(distance);
		if (m_Reset) {
			m_SmellWME = m_Agent.CreateIdWME(m_InputLink, kSmellID);
			m_SmellColorWME = CreateStringWME(m_SmellWME, kColorID, closestTankColor);
			if (closestTank == null) {
				m_SmellDistanceWME = null;
				m_SmellDistanceStringWME = CreateStringWME(m_SmellWME, kDistanceID, kNone);
			} else {
				m_SmellDistanceWME = CreateIntWME(m_SmellWME, kDistanceID, distance);
				m_SmellDistanceStringWME = null;
			}
		} else {
			if (!m_SmellColorWME.GetValue().equalsIgnoreCase(closestTankColor)) {
				Update(m_SmellColorWME, closestTankColor);
			}
			if (closestTank == null) {
				if (m_SmellDistanceWME != null) {
					DestroyWME(m_SmellDistanceWME);
					m_SmellDistanceWME = null;
				}
				if (m_SmellDistanceStringWME == null) {
					m_SmellDistanceStringWME = CreateStringWME(m_SmellWME, kDistanceID, kNone);
				}
			} else {
				if (m_SmellDistanceWME == null) {
					m_SmellDistanceWME = CreateIntWME(m_SmellWME, kDistanceID, distance);
				} else {
					if (m_SmellDistanceWME.GetValue() != distance) {
						Update(m_SmellDistanceWME, distance);
					}
				}
				if (m_SmellDistanceStringWME != null) {
					DestroyWME(m_SmellDistanceStringWME);
					m_SmellDistanceStringWME = null;
				}
			}
		}

		// Missiles
		int missiles = m_Tank.getMissiles();
		if (m_Reset) {
			m_MissilesWME = CreateIntWME(m_InputLink, kMissilesID, missiles);
		} else {
			if (m_MissilesWME.GetValue() != missiles) {
				Update(m_MissilesWME, missiles);
			}
		}
		
		// Color
		if (m_Reset) {
			CreateStringWME(m_InputLink, kMyColorID, m_Tank.getColor());
		}
		
		int worldCount = m_World.getWorldCount();
		if (m_Reset) {
			m_ClockWME = CreateIntWME(m_InputLink, kClockID, worldCount);
		} else {
			Update(m_ClockWME, worldCount);
		}
		
		// Resurrect
		if (m_Reset) {
			m_ResurrectFrame = worldCount;
			m_ResurrectWME = CreateStringWME(m_InputLink, kResurrectID, kYes);
		} else {
			if (worldCount != m_ResurrectFrame) {
				if (!m_ResurrectWME.GetValue().equalsIgnoreCase(kNo)) {
					Update(m_ResurrectWME, kNo);
				}
			}
		}
		
		// Radar
		String radarStatus = m_Tank.getRadarStatus() ? kOn : kOff;
		if (m_Reset) {
			m_RadarStatusWME = CreateStringWME(m_InputLink, kRadarStatusID, radarStatus);
			if (m_Tank.getRadarStatus()) {
				m_RadarWME = m_Agent.CreateIdWME(m_InputLink, kRadarID);
				generateNewRadar();
			} else {
				m_RadarWME = null;
			}
			m_RadarDistanceWME = CreateIntWME(m_InputLink, kRadarDistanceID, m_Tank.getRadarDistance());
			m_RadarSettingWME = CreateIntWME(m_InputLink, kRadarSettingID, m_Tank.getRadarSetting());
			
		} else {
			if (!m_RadarStatusWME.GetValue().equalsIgnoreCase(radarStatus)) {
				Update(m_RadarStatusWME, radarStatus);
			}
			if (m_Tank.getRadarStatus()) {
				if (m_RadarWME == null) {
					m_RadarWME = m_Agent.CreateIdWME(m_InputLink, kRadarID);
					generateNewRadar();
				} else {
					updateRadar();
				}
			} else {
				if (m_RadarWME != null) {
					DestroyWME(m_RadarWME);
					m_RadarWME = null;
					clearRadar();
				}
			}
			if (m_RadarDistanceWME.GetValue() != m_Tank.getRadarDistance()) {
				Update(m_RadarDistanceWME, m_Tank.getRadarDistance());
			}
			if (m_RadarSettingWME.GetValue() != m_Tank.getRadarSetting()) {
				Update(m_RadarSettingWME, m_Tank.getRadarSetting());
			}
		}
		
		// Random
		float random = m_World.m_Random.nextFloat();
		if (m_Reset) {
			m_RandomWME = CreateFloatWME(m_InputLink, kRandomID, random);
		} else {
			Update(m_RandomWME, random);
		}

		// RWaves
		int rwaves = m_Tank.getRWaves();
		String rwavesForward = (rwaves & m_Tank.forward()) > 0 ? kYes : kNo;
		String rwavesBackward = (rwaves & m_Tank.backward()) > 0 ? kYes : kNo;;
		String rwavesLeft = (rwaves & m_Tank.left()) > 0 ? kYes : kNo;
		String rwavesRight = (rwaves & m_Tank.right()) > 0 ? kYes : kNo;
		
		if (m_Reset) {
			m_RWavesWME = m_Agent.CreateIdWME(m_InputLink, kRWavesID);
			m_RWavesForwardWME = CreateStringWME(m_RWavesWME, kForwardID, rwavesBackward);
			m_RWavesBackwardWME = CreateStringWME(m_RWavesWME, kBackwardID, rwavesForward);
			m_RWavesLeftWME = CreateStringWME(m_RWavesWME, kLeftID, rwavesLeft);
			m_RWavesRightWME = CreateStringWME(m_RWavesWME, kRightID, rwavesRight);
		} else {
			if (!m_RWavesForwardWME.GetValue().equalsIgnoreCase(rwavesForward)) {
				Update(m_RWavesForwardWME, rwavesForward);
			}
			if (!m_RWavesBackwardWME.GetValue().equalsIgnoreCase(rwavesBackward)) {
				Update(m_RWavesBackwardWME, rwavesBackward);
			}
			if (!m_RWavesLeftWME.GetValue().equalsIgnoreCase(rwavesLeft)) {
				Update(m_RWavesLeftWME, rwavesLeft);
			}
			if (!m_RWavesRightWME.GetValue().equalsIgnoreCase(rwavesRight)) {
				Update(m_RWavesRightWME, rwavesRight);
			}
			
		}	
		
		m_Reset = false;
		m_Agent.Commit();
	}
	
	private void generateNewRadar() {
//		m_Logger.log("generateNewRadar()");
		TankSoarCell[][] radarCells = m_Tank.getRadarCells();
		for (int j = 0; j < Tank.kRadarHeight; ++j) {
			boolean done = false;
//			String outstring = new String();
			for (int i = 0; i < Tank.kRadarWidth; ++i) {
				// Always skip self, this screws up the tanks.
				if (i == 1 && j == 0) {
//					outstring += "s";
					continue;
				}
				if (radarCells[i][j] == null) {
					// if center is null, we're done
					if (i == 1) {
//						outstring += "d";
						done = true;
						break;
					} else {
//						outstring += ".";
					}
				} else {
					// Create a new WME
					radarCellIDs[i][j] = m_Agent.CreateIdWME(m_RadarWME, getCellID(radarCells[i][j]));
					CreateIntWME(radarCellIDs[i][j], kDistanceID, j);
					CreateStringWME(radarCellIDs[i][j], kPositionID, getPositionID(i));
					if (radarCells[i][j].containsTank()) {
						radarColors[i][j] = CreateStringWME(radarCellIDs[i][j], kColorID, radarCells[i][j].getTank().getColor());
//						outstring += "t";
					} else {
//						outstring += "n";
					}
				}
			}
//			m_Logger.log(outstring);
			if (done == true) {
				break;
			}
		}
	}
	
	private void updateRadar() {
//		m_Logger.log("updateRadar()");
		TankSoarCell[][] radarCells = m_Tank.getRadarCells();
		for (int i = 0; i < Tank.kRadarWidth; ++i) {
//			String outstring = new String();
			for (int j = 0; j < Tank.kRadarHeight; ++j) {
				// Always skip self, this screws up the tanks.
				if (i == 1 && j == 0) {
//					outstring += "s";
					continue;
				}
				if (radarCells[i][j] == null) {
					// Unconditionally delete the WME
					if (radarCellIDs[i][j] != null) {
//						outstring += "d";
						DestroyWME(radarCellIDs[i][j]);
						radarCellIDs[i][j] = null;
						radarColors[i][j] = null;
					} else {
//						outstring += "-";
					}
					
				} else {
					
					if (radarCellIDs[i][j] == null) {
						// Unconditionally create the WME
						radarCellIDs[i][j] = m_Agent.CreateIdWME(m_RadarWME, getCellID(radarCells[i][j]));
						CreateIntWME(radarCellIDs[i][j], kDistanceID, j);
						CreateStringWME(radarCellIDs[i][j], kPositionID, getPositionID(i));
						if (radarCells[i][j].containsTank()) {
							radarColors[i][j] = CreateStringWME(radarCellIDs[i][j], kColorID, radarCells[i][j].getTank().getColor());
//							outstring += "t";
						} else {
//							outstring += "n";
						}
					} else {
						// Update if relevant change
						if (m_Tank.recentlyMovedOrRotated() || radarCells[i][j].isModified()) {
							DestroyWME(radarCellIDs[i][j]);
							radarCellIDs[i][j] = m_Agent.CreateIdWME(m_RadarWME, getCellID(radarCells[i][j]));
							CreateIntWME(radarCellIDs[i][j], kDistanceID, j);
							CreateStringWME(radarCellIDs[i][j], kPositionID, getPositionID(i));
							if (radarCells[i][j].containsTank()) {
								// rwaves already set
								//tank.setRWaves(m_Tank.backward());
								radarColors[i][j] = CreateStringWME(radarCellIDs[i][j], kColorID, radarCells[i][j].getTank().getColor());
//								outstring += "U";
							} else {
//								outstring += "u";								
							}
						} else {
//							outstring += ".";							
						}
					}
				}
			}
//			m_Logger.log(outstring);
		}
	}

	private void clearRadar() {
//		m_Logger.log("clearRadar()");
		for (int i = 0; i < Tank.kRadarWidth; ++i) {
			for (int j = 0; j < Tank.kRadarHeight; ++j) {
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
	
	public String getPositionID(int i) {
		switch (i) {
		case Tank.kRadarLeft:
			return InputLinkManager.kLeftID;
		default:
		case Tank.kRadarCenter:
			return InputLinkManager.kCenterID;
		case Tank.kRadarRight:
			return InputLinkManager.kRightID;
		}
	}
}

















