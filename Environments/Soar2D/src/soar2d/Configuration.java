package soar2d;

import java.util.*;
import java.util.logging.*;
import java.io.*;

import soar2d.player.*;

/**
 * @author voigtjr
 * 
 * This class holds all the configuration for the current simulation.
 * Not all fields are applicable for all types of simulations.
 */
public class Configuration {
	
	public final String kDefaultXMLEatersSettingsFile = "eaters.xml";
	public final String kDefaultXMLTankSoarSettingsFile = "tanksoar.xml";
	public final String kDefaultXMLEatersConsoleSettingsFile = "eaters-console.xml";
	public final String kDefaultXMLTankSoarConsoleSettingsFile = "tanksoar-console.xml";
	public final String kDefaultXMLBookSettingsFile = "book.xml";
	public final String kDefaultLogFilename = "soar2d.log";
	public final String kMapDir = "maps";
	public final String kAgentDir = "agents";
	public final String kDefaultEatersMap = "random-walls.emap";
	public final String kDefaultTanksoarMap = "default.tmap";
	public final String kDefaultBookMap = "default.bmap";
	public final String kEatersMapExt = "emap";
	public final String kTankSoarMapExt = "tmap";
	public final int kDefaultTimeout = 15; //seconds
	public final int kDefaultVersion = 1;
	public final boolean kDefaultPropertyBoolean = false;	// if a bool property doesn't exist
	public final float kDefaultPropertyFloat = 0;			// if a float property doesn't exist
	public final int kDefaultPropertyInt = 0;				// if an int property doesn't exist

	// private data:
	private String basePath = null; // Path to Soar2D folder
	private String mapPath = null; // Path to maps
	private String agentPath = null; // Path to agents
	private SimType simType = SimType.kEaters;
	public enum SimType { kEaters, kTankSoar, kBook }
	
	// common
	public int defaultPoints = 0;
	public Level logLevel = Level.INFO; // Current log level, see java.util.logging
	public boolean logToFile = true; //
	public File logFile = new File(kDefaultLogFilename); // File to log to, only valid if logFile true
	public boolean logTime = false;
	public boolean logConsole = false; // If true, log to console
	public int randomSeed = 0;
	public boolean graphical = true; // If true, use the window manager.
	public int asyncTimeSlice = 0; // If positive, run soar asynchronously
	public boolean debuggers = true; // If true, spawn debuggers
	public boolean remote = false; // If true, connect to remote kernel
	public boolean random = true; // If true, use random numbers (don't seed generators)
	public boolean noWorld = false; // If true, do not display the world map
	public File map = null; // The current map file
	public ArrayList<ClientConfig> clients = new ArrayList<ClientConfig>(); // List of clients that will connect to the simulation
	public ArrayList<PlayerConfig> players = new ArrayList<PlayerConfig>(); // List of information required to create players
	
	// eaters-specific
	public int eaterVision = 2;	// distance that eater can see
	public int wallPenalty = -5;	// eaters scoring penalty
	public int jumpPenalty = -5;	// eaters scoring penalty
	public double lowProbability = .15;		// eaters wall generation
	public double higherProbability = .65;	// eaters wall generation
	
	// tanksoar-specific
	public int defaultMissiles = 15;
	public int defaultEnergy = 1000;
	public int defaultHealth = 1000;
	public int tankCollisionPenalty = -100;	// tanksoar
	public int maxMissilePacks = 3;
	public int missilePackRespawnChance = 5;	// percent chance per update that a missile pack will respawn
	public int sheildEnergyUsage = -20;
	public int missileHitAward = 2;
	public int missileHitPenalty = -1;
	public int killAward = 3;
	public int killPenalty = -2;
	public int radarWidth = 3;
	public int radarHeight = 15;
	public int maxSmellDistance = 7;
	public int missileResetThreshold = 100;
	
	// Terminal triggers
	/**
	 * Stop the simulation when no points remain
	 */
	public boolean terminalPointsRemaining = false;
	/**
	 * Stop the simulation when no food remains
	 */
	public boolean terminalFoodRemaining = false;
	/**
	 * Stop the simulation when no boxes are closed
	 */
	public boolean terminalUnopenedBoxes = false;
	/**
	 * Stop the simulation when an agent issues a stop command.
	 * Note that a similar but slightly different effect can be achieved with 
	 * right hand side "cmd stop-soar"
	 */
	public boolean terminalAgentCommand = false;
	/**
	 * Stop the simulation when a certain number of world updates have completed.
	 * Zero indicates do not use this terminal condition
	 */
	public int terminalMaxUpdates = 0;
	/**
	 * Stop when one or more agents has this score or more
	 * Must be positive for this terminal condition to work
	 */
	public int terminalWinningScore = 0;
	
	public Configuration() {}
	
	public void setType(SimType simType) {
		this.simType = simType;
		generatePaths();
	}
	public SimType getType() {
		return this.simType;
	}
	public String getBasePath() {
		assert basePath != null;
		return new String(basePath);
	}
	public String getMapPath() {
		assert mapPath != null;
		return new String(mapPath);
	}
	public String getAgentPath() {
		assert agentPath != null;
		return new String(agentPath);
	}
	
	public void setDefaultTerminals() {
		switch (simType) {
		case kEaters:
			this.terminalWinningScore = 0;
			this.terminalFoodRemaining = true;
			break;
		case kTankSoar:
			this.terminalWinningScore = 50;
			this.terminalFoodRemaining = false;
			break;
		case kBook:
			assert false;
			this.terminalWinningScore = 0;
			this.terminalFoodRemaining = false;
			break;
		}
	}

	private void generatePaths() {
		
		this.basePath = System.getProperty("user.dir") + System.getProperty("file.separator");
		this.mapPath = this.basePath + this.kMapDir + System.getProperty("file.separator");
		this.agentPath = this.basePath + this.kAgentDir + System.getProperty("file.separator");
		switch (simType) {
		case kEaters:
			this.mapPath += "eaters" + System.getProperty("file.separator");
			this.agentPath += "eaters" + System.getProperty("file.separator");
			this.map = new File(this.mapPath + this.kDefaultEatersMap);
			break;
		case kTankSoar:
			this.mapPath += "tanksoar" + System.getProperty("file.separator");
			this.agentPath += "tanksoar" + System.getProperty("file.separator");
			this.map = new File(this.mapPath + this.kDefaultTanksoarMap);
			break;
		case kBook:
			this.mapPath += "book" + System.getProperty("file.separator");
			this.agentPath += "book" + System.getProperty("file.separator");
			this.map = new File(this.mapPath + this.kDefaultBookMap);
			break;
		}
	}
	
	public Configuration(Configuration config) {
		this.simType = config.simType;

		// common
		this.defaultPoints = config.defaultPoints;
		this.logLevel = Level.parse(config.logLevel.getName());
		this.logToFile = config.logToFile;
		this.logFile = config.logFile.getAbsoluteFile();
		this.logTime = config.logTime;
		this.logConsole = config.logConsole;
		this.randomSeed = config.randomSeed;
		this.graphical = config.graphical;
		this.debuggers = config.debuggers;
		this.remote = config.remote;
		this.random = config.random;
		this.noWorld = config.noWorld;
		this.map = config.map.getAbsoluteFile();
		this.clients = new ArrayList<ClientConfig>(config.clients);
		this.players = new ArrayList<PlayerConfig>(config.players);
		this.basePath = new String(config.basePath);
		this.mapPath = new String(config.mapPath);
		this.agentPath = new String(config.agentPath);
		
		// eaters-specific
		this.eaterVision = config.eaterVision;
		this.wallPenalty = config.wallPenalty;
		this.jumpPenalty = config.jumpPenalty;
		this.lowProbability = config.lowProbability;
		this.higherProbability = config.higherProbability;
		
		// tanksoar-specific
		this.defaultMissiles = config.defaultMissiles;
		this.defaultEnergy = config.defaultEnergy;
		this.defaultHealth = config.defaultHealth;
		this.tankCollisionPenalty = config.tankCollisionPenalty;
		this.maxMissilePacks = config.maxMissilePacks;
		this.missilePackRespawnChance = config.missilePackRespawnChance;
		this.sheildEnergyUsage = config.sheildEnergyUsage;
		this.missileHitAward = config.missileHitAward;
		this.missileHitPenalty = config.missileHitPenalty;
		this.killAward = config.killAward;
		this.killPenalty = config.killPenalty;
		this.radarWidth = config.radarWidth;
		this.radarHeight = config.radarHeight;
		this.maxSmellDistance = config.maxSmellDistance;
		this.missileResetThreshold = config.missileResetThreshold;
		
		this.terminalPointsRemaining = config.terminalPointsRemaining;
		this.terminalFoodRemaining = config.terminalFoodRemaining;
		this.terminalUnopenedBoxes = config.terminalUnopenedBoxes;
		this.terminalAgentCommand = config.terminalAgentCommand;
		this.terminalMaxUpdates = config.terminalMaxUpdates;
		this.terminalWinningScore = config.terminalWinningScore;
	}
}
