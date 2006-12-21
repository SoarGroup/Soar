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
	public final String kDefaultLogFilename = "soar2d.log";
	public final String kMapDir = "maps";
	public final String kAgentDir = "agents";
	public final String kEatersMapFilter = "*.emap";
	public final String kTankSoarMapFilter = "*.emap";

	public final int kDefaultPoints = 0;
	public final int kDefaultMissiles = 0;
	public final int kDefaultEnergy = 1000;
	public final int kDefaultHealth = 1000;
	public final int kDefaultVersion = 1;
	public final int kDefaultTimeout = 15; //seconds
	public final Level kDefaultLogLevel = Level.INFO;
	public final int kEaterVision = 2;	// distance that eater can see
	public final int kWallPenalty = -5;	// eaters scoring penalty
	public final int kJumpPenalty = -5;	// eaters scoring penalty
	public final double kLowProbability = .15;		// eaters wall generation
	public final double kHigherProbability = .65;	// eaters wall generation
	public final int kTankCollisionPenalty = -100;	// tanksoar
	public final int kMaxMissilePacks = 3;
	public final int kMissilePackRespawnChance = 5;	// percent chance per update that a missile pack will respawn

	public final boolean kDefaultPropertyBoolean = false;	// if a bool property doesn't exist
	public final float kDefaultPropertyFloat = 0;			// if a float property doesn't exist
	public final int kDefaultPropertyInt = 0;				// if an int property doesn't exist
	
	/**
	 * Current log level, see java.util.logging
	 */
	public Level logLevel = kDefaultLogLevel;
	/**
	 * If true, log to file logFileName
	 */
	public boolean logFile = false;
	/**
	 * File to log to, only valid if logFile true
	 */
	public String logFileName = null;
	/**
	 * If true, log to console
	 */
	public boolean logConsole = false;
	/**
	 * If true, run eaters-specific code, mutually exclusive with tanksoar
	 */
	public boolean eaters = false;
	/**
	 * If true, run tanksoar-specific code, mutually exclusive with eaters
	 */
	public boolean tanksoar = false;
	/**
	 * If true, use the window manager.
	 */
	public boolean graphical = true;
	/**
	 * If true, spawn debuggers
	 */
	public boolean debuggers = false;
	/**
	 * If true, connect to remote kernel
	 */
	public boolean remote = false;
	/**
	 * If true, use random numbers (don't seed generators)
	 */
	public boolean random = true;
	/**
	 * The current map file
	 */
	public File map = null;
	/**
	 * List of clients that will connect to the simulation
	 */
	public ArrayList<ClientConfig> clients = new ArrayList<ClientConfig>();
	/**
	 * List of information required to create players
	 */
	public ArrayList<PlayerConfig> players = new ArrayList<PlayerConfig>();
	/**
	 * Path to Soar2D folder
	 */
	public String basePath = null;
	/**
	 * Path to maps
	 */
	public String mapPath = null;
	/**
	 * Path to agents
	 */
	public String agentPath = null;

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
}
