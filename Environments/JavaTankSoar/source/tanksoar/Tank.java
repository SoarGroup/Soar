package tanksoar;

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
	
	public final static String kEast = "east";
	public final static String kNorth = "north";
	public final static String kSouth = "south";
	public final static String kWest = "west";
	
	private final static String kYes = "yes";
	private final static String kNo = "no";
	
	private final static String kOff = "off";
	private final static String kOn = "on";
	
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
	
	private final static int kInitialEnergy = 1000;
	private final static int kInitialHealth = 1000;
	private final static int kInitialMissiles = 15;
	
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
	private StringElement m_SoundWME;
	private IntElement m_xWME;
	private IntElement m_yWME;
	
	public Tank(Agent agent, String productions, String color, Point location) {
		super(agent, productions, color, location);
		
		Identifier inputLink = m_Agent.GetInputLink();
		
		Identifier blocked = m_Agent.CreateIdWME(inputLink, kBlockedID);
		m_BlockedBackwardWME = m_Agent.CreateStringWME(blocked, kBackwardID, kNo);
		m_BlockedForwardWME = m_Agent.CreateStringWME(blocked, kForwardID, kNo);
		m_BlockedLeftWME = m_Agent.CreateStringWME(blocked, kLeftID, kNo);
		m_BlockedRightWME = m_Agent.CreateStringWME(blocked, kRightID, kNo);
		
		// TODO: initial clock setting depends on world count
		m_ClockWME = m_Agent.CreateIntWME(inputLink, kClockID, 1);
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
		// TODO: Substructure depends on situation
		
		m_RadarDistanceWME = m_Agent.CreateIntWME(inputLink, kRadarDistanceID, 0);
		m_RadarSettingWME = m_Agent.CreateIntWME(inputLink, kRadarSettingID, 0);
		m_RadarStatusWME = m_Agent.CreateStringWME(inputLink, kRadarStatusID, kOff);
		m_RandomWME = m_Agent.CreateFloatWME(inputLink, kRandomID, 0);
		m_ResurrectWME = m_Agent.CreateStringWME(inputLink, kResurrectID, kNo);
		
		Identifier rwaves = m_Agent.CreateIdWME(inputLink, kRWavesID);
		m_RWavesBackwardWME = m_Agent.CreateStringWME(rwaves, kBackwardID, kNo);
		m_RWavesForwardWME = m_Agent.CreateStringWME(rwaves, kForwardID, kNo);
		m_RWavesLeftWME = m_Agent.CreateStringWME(rwaves, kLeftID, kNo);
		m_RWavesRightWME = m_Agent.CreateStringWME(rwaves, kRightID, kNo);
		
		m_ShieldStatusWME = m_Agent.CreateStringWME(inputLink, kShieldStatusID, kOff);
		
		m_SmellWME = m_Agent.CreateIdWME(inputLink, kSmellID);
		// TODO: Substructure depends on situation

		m_SoundWME = m_Agent.CreateStringWME(inputLink, kSoundID, kSilentID);
		m_xWME = m_Agent.CreateIntWME(inputLink, kXID, location.x);
		m_yWME = m_Agent.CreateIntWME(inputLink, kYID, location.y);
						
		m_Agent.Commit();		
	}
	
	public void updateInput(TankSoarWorld world) {
		
	}
	
	public MoveInfo getMove() {
		return m_LastMove;
	}
}
