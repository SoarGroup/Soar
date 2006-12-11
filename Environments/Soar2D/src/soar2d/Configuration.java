package soar2d;

import java.util.*;
import java.util.logging.*;
import java.io.*;

import soar2d.player.*;

class AnalyzeError extends Exception {
	public static final long serialVersionUID = 1;
	public AnalyzeError(String message) {
		super(message);
	}
}

public class Configuration {
	
	public final String kDefaultXMLEatersSettingsFile = "eaters.xml";
	public final String kDefaultXMLTankSoarSettingsFile = "tanksoar.xml";
	public final String kDefaultLogFilename = "soar2d.log";
	public final String kMapDir = "maps";
	public final String kAgentDir = "agents";
	public final String kMapFilter = "*.emap";

	public final int kDefaultPoints = 0;
	public final int kDefaultVersion = 1;
	public final int kDefaultTimeout = 15; //seconds
	public final Level kDefaultLogLevel = Level.INFO;
	public final int kEaterVision = 2;
	public final int kWallPenalty = -5;	// eaters scoring penalty
	public final int kJumpPenalty = -5;	// eaters scoring penalty
	public final double kLowProbability = .15;		// eaters wall generation
	public final double kHigherProbability = .65;	// eaters wall generation

	public final boolean kDefaultPropertyBoolean = false;
	public final float kDefaultPropertyFloat = 0;
	public final int kDefaultPropertyInt = 0;
	
	public Level logLevel = kDefaultLogLevel;
	public boolean logFile = false;
	public String logFileName = null;
	public boolean logConsole = false;
	public boolean eaters = false;
	public boolean tanksoar = false;
	public boolean graphical = true;
	public boolean debuggers = false;
	public boolean remote = false;
	public boolean random = true;
	public File map = null;
	public ArrayList<ClientConfig> clients = new ArrayList<ClientConfig>();
	public ArrayList<PlayerConfig> players = new ArrayList<PlayerConfig>();
	public String basePath = null;
	public String mapPath = null;
	public String agentPath = null;

	public boolean terminalPointsRemaining = false;
	public boolean terminalFoodRemaining = false;
	public boolean terminalAgentCommand = false;
	public int terminalMaxUpdates = 0;
	public int terminalWinningScore = 0;

}
