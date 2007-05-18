package soar2d;

import soar2d.ClientConfig;

/**
 * @author voigtjr
 *
 * Useful constants
 */
public class Names {
	public static final String kNone = "none";

	public static final String kTrue = "true";
	public static final String kFalse = "false";

	public static final String kNorth = "north";
	public static final String kEast = "east";
	public static final String kSouth = "south";
	public static final String kWest = "west";
	
	public static final String kYes = "yes";
	public static final String kNo = "no";
	
	public static final String kRotateLeft = "left";
	public static final String kRotateRight = "right";
	public static final String kRotateStop = "stop";
	
	public static final String kHumanProductions = "<Human>";
	
	public static final String kEmpty = "empty";
	public static final String kEnergy = "energy";
	public static final String kExplosion = "explosion";
	public static final String kGround = "ground";
	public static final String kHealth = "health";
	public static final String kMissiles = "missiles";
	public static final String kOpen = "open";
	public static final String kRedraw = "redraw";
	
	public static final String kBackwardID = "backward";
	public static final String kBlockedID = "blocked";
	public static final String kCenterID = "center";
	public static final String kClockID = "clock";
	public static final String kColorID = "color";
	public static final String kContentID = "content";
	public static final String kCurrentScoreID = "current-score";
	public static final String kDirectionID = "direction";
	public static final String kDistanceID = "distance";
	public static final String kDontEatID = "dont-eat";
	public static final String kGatewayID = "gateway";
	public static final String kEaterID = "eater";
	public static final String kEnergyID = "energy";
	public static final String kEnergyRechargerID = "energyrecharger";
	public static final String kFireID = "fire";
	public static final String kFloatXID = "floatx";
	public static final String kFloatYID = "floaty";
	public static final String kForwardID = "forward";
	public static final String kHeadingID = "heading";
	public static final String kHealthID = "health";
	public static final String kHealthRechargerID = "healthrecharger";
	public static final String kIdID = "id";
	public static final String kInID = "in";
	public static final String kIncomingID = "incoming";
	public static final String kJumpID = "jump";
	public static final String kLeftID = "left";
	public static final String kMissilesID = "missiles";
	public static final String kMoveID = "move";
	public static final String kMyColorID = "my-color";
	public static final String kMyLocationID = "my-location";
	public static final String kNameID = "name";
	public static final String kObstacleID = "obstacle";
	public static final String kOff = "off";
	public static final String kOn = "on";
	public static final String kOpenID = "open";
	public static final String kOpenCodeID = "open-code";
	public static final String kPositionID = "position";
	public static final String kRadarID = "radar";
	public static final String kRadarPowerID = "radar-power";
	public static final String kRandomID = "random";
	public static final String kRadarDistanceID = "radar-distance";
	public static final String kRadarSettingID = "radar-setting";
	public static final String kRadarStatusID = "radar-status";
	public static final String kResurrectID = "resurrect";
	public static final String kRightID = "right";
	public static final String kRoomID = "room";
	public static final String kRotateID = "rotate";
	public static final String kRotateAbsoluteID = "rotate-absolute";
	public static final String kRotateRelativeID = "rotate-relative";
	public static final String kRWavesID = "rwaves";
	public static final String kScoreID = "score";
	public static final String kSettingID = "setting";
	public static final String kShieldsID = "shields";
	public static final String kShieldStatusID = "shield-status";
	public static final String kSilentID = "silent";
	public static final String kSmellID = "smell";
	public static final String kSoundID = "sound";
	public static final String kStopID = "stop";
	public static final String kStopSimID = "stop-sim";
	public static final String kSwitchID = "switch";
	public static final String kTankID = "tank";
	public static final String kWallID = "wall";
	public static final String kXID = "x";
	public static final String kYID = "y";
	
	public static final String kUnnamedCellObject = "Unnamed CellObject";
	
	public static final String kPropertyBlock = "block";
	public static final String kPropertyBox = "box";
	public static final String kPropertyBoxID = "box-id";
	public static final String kPropertyCharger = "charger";
	public static final String kPropertyColor = "color";
	public static final String kPropertyDirection = "direction";
	public static final String kPropertyGateway = "gateway";
	public static final String kPropertyGatewayRender = "gateway-render";
	public static final String kPropertyEdible = "edible";
	public static final String kPropertyEnergy = "energy";
	public static final String kPropertyFlyPhase = "fly-phase";
	public static final String kPropertyHealth = "health";
	public static final String kPropertyID = "id";
	public static final String kPropertyImage = "image";
	public static final String kPropertyImageMin = "image-min";
	public static final String kPropertyImageMax = "image-max";
	public static final String kPropertyLinger = "linger";
	public static final String kPropertyMiniImage = "mini-image";
	public static final String kPropertyMissile = "missile";
	public static final String kPropertyMissiles = "missiles";
	public static final String kPropertyMoveApply = "move-apply";
	public static final String kPropertyNumber = "number";
	public static final String kPropertyOpenCode = "open-code";
	public static final String kPropertyOwner = "owner";
	public static final String kPropertyPoints = "points";
	public static final String kPropertyPositiveBoxID = "positive-box-id";
	public static final String kPropertyRadarWaves = "radar-waves";
	public static final String kPropertyRoom = "room";
	public static final String kPropertyShape = "shape";
	public static final String kPropertyStatus = "status";

	public static final ClientConfig kDebuggerClient = new ClientConfig();

	static {
		kDebuggerClient.name = "java-debugger";
		kDebuggerClient.after = true;
		kDebuggerClient.command = null;
		kDebuggerClient.timeout = ClientConfig.kDefaultTimeout;
	}
}
