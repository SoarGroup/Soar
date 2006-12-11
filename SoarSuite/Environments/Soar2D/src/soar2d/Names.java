package soar2d;

public class Names {
	public static final String kTrue = "true";
	public static final String kFalse = "false";

	public static final String kNorth = "north";
	public static final String kEast = "east";
	public static final String kSouth = "south";
	public static final String kWest = "west";
	
	public static final String kHumanProductions = "<Human>";
	
	public static final String kEmpty = "empty";
	public static final String kExplosion = "explosion";
	public static final String kRedraw = "redraw";
	public static final String kOpen = "open";
	public static final String kWall = "wall";
	
	public static final String kTagAgent = "agent";
	public static final String kTagApply = "apply";
	public static final String kTagCellObject = "cell-object";
	public static final String kTagCell = "cell";
	public static final String kTagCells = "cells";
	public static final String kTagClient = "client";
	public static final String kTagDecay = "decay";
	public static final String kTagDisplay = "display";
	public static final String kTagEaters = "eaters";
	public static final String kTagEnergy = "energy";
	public static final String kTagFood = "food";
	public static final String kTagHealth = "health";
	public static final String kTagLogger = "logger";
	public static final String kTagMap = "map";
	public static final String kTagMissiles = "missiles";
	public static final String kTagObject = "object";
	public static final String kTagPoints = "points";
	public static final String kTagProperty = "property";
	public static final String kTagRow = "row";
	public static final String kTagShutdownCommand = "shutdown-command";
	public static final String kTagSimulation = "simulation";
	public static final String kTagSoar2D = "soar2d";
	public static final String kTagTankSoar = "tanksoar";
	public static final String kTagTerminal = "terminal";
	public static final String kTagUpdate = "update";

	public static final String kParamAfter = "after";
	public static final String kParamColor = "color";
	public static final String kParamCommand = "command";
	public static final String kParamConsole = "console";
	public static final String kParamConsumable = "consumable";
	public static final String kParamDebuggers = "debuggers";
	public static final String kParamDecay = "decay";
	public static final String kParamEnergy = "energy";
	public static final String kParamFacing = "facing";
	public static final String kParamFile = "file";
	public static final String kParamFileName = "filename";
	public static final String kParamGraphical = "graphical";
	public static final String kParamHealth = "health";
	public static final String kParamLevel = "level";
	public static final String kParamMap = "map";
	public static final String kParamMissiles = "missiles";
	public static final String kParamName = "name";
	public static final String kParamProductions = "productions";
	public static final String kParamProperty = "property";
	public static final String kParamRandom = "random";
	public static final String kParamRandomFood = "random-food";
	public static final String kParamRandomWalls = "random-walls";
	public static final String kParamRemote = "remote";
	public static final String kParamShape = "shape";
	public static final String kParamTimeOut = "timeout";
	public static final String kParamType = "type";
	public static final String kParamUpdatable = "updatable";
	public static final String kParamValue = "value";
	public static final String kParamVersion = "version";
	public static final String kParamWorldSize = "world-size";
	public static final String kParamX = "x";
	public static final String kParamY = "y";
	
	public static final String kTerminalMaxUpdates = "max-updates";
	public static final String kTerminalAgentCommand = "agent-command";
	public static final String kTerminalPointsRemaining = "points-remaining";
	public static final String kTerminalWinningScore = "winning-score";
	public static final String kTerminalFoodRemaining = "food-remaining";

	public static final String kLevelSevere = "severe";
	public static final String kLevelWarning = "warning";
	public static final String kLevelInfo = "info";
	public static final String kLevelFine = "fine";
	public static final String kLevelFiner = "finer";
	public static final String kLevelFinest = "finest";

	public static final String kBoxID = "box";
	public static final String kContentID = "content";
	public static final String kEater = "eater";
	public static final String kEaterID = "eater";
	public static final String kDirectionID = "direction";
	public static final String kDontEatID = "dont-eat";
	public static final String kJumpID = "jump";
	public static final String kMoveID = "move";
	public static final String kMyLocationID = "my-location";
	public static final String kNameID = "name";
	public static final String kOpenID = "open";
	public static final String kRandomID = "random";
	public static final String kScoreID = "score";
	public static final String kStopID = "stop";
	public static final String kxID = "x";
	public static final String kyID = "y";
	
	public static final String kUnnamedCellObject = "Unnamed CellObject";
	
	public static final String kPropertyBlock = "block";
	public static final String kPropertyBox = "box";
	public static final String kPropertyColor = "color";
	public static final String kPropertyEdible = "edible";
	public static final String kPropertyMoveApply = "move-apply";
	public static final String kPropertyPoints = "points";
	public static final String kPropertyShape = "shape";
	public static final String kPropertyStatus = "status";

	public static final ClientConfig kDebuggerClient = new ClientConfig();
	static {
		kDebuggerClient.name = "java-debugger";
		kDebuggerClient.after = true;
		kDebuggerClient.command = null;
		kDebuggerClient.timeout = Soar2D.config.kDefaultTimeout;
	}
}
