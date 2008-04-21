package soar2d.configuration;

import java.util.*;
import java.util.logging.*;
import java.io.*;

import org.jdom.Document;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;
import org.jdom.Element;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

import soar2d.ClientConfig;
import soar2d.Direction;
import soar2d.Soar2D;
import soar2d.player.*;

/**
 * @author voigtjr
 * 
 * This class holds all the configuration for the current simulation.
 * Not all fields are applicable for all types of simulations.
 */
public class Configuration {
	
	public TankSoarConfiguration tConfig;
	public EatersConfiguration eConfig;
	public BookConfiguration bConfig;
	public KitchenConfiguration kConfig;
	public TaxiConfiguration xConfig;

	// misc non-configurable configuration

	public Configuration() {}
	
	public Configuration(Configuration config) {
		
		// meta
		this.setType(config.simType);
		this.setMap(config.getMap().getAbsoluteFile());
		this.port = config.port;
		this.toscaEnabled = config.toscaEnabled;

		// general
		this.random = config.random;
		this.randomSeed = config.randomSeed;
		this.remote = config.remote;
		this.hide = config.hide;
		this.nogui = config.nogui;
		this.silentAgents = config.silentAgents;
		this.maxMemoryUsageValue = config.maxMemoryUsageValue;
		this.asyncTimeSlice = config.asyncTimeSlice;
		if (config.metadata != null) {
			this.metadata = new File(config.metadata.getAbsolutePath());
		}

		cModule.copy(cModule);

		// logging
		this.logLevel = Level.parse(config.logLevel.getName());
		this.logFile = config.logFile.getAbsoluteFile();
		this.logTime = config.logTime;
		this.logConsole = config.logConsole;

		// players
		this.defaultPoints = config.defaultPoints;
		this.debuggers = config.debuggers;
		this.players = new ArrayList<PlayerConfig>(config.players);
		
		// terminals
		this.terminalPointsRemaining = config.terminalPointsRemaining;
		this.terminalFoodRemaining = config.terminalFoodRemaining;
		this.terminalFoodRemainingContinue = config.terminalFoodRemainingContinue;
		this.terminalUnopenedBoxes = config.terminalUnopenedBoxes;
		this.terminalAgentCommand = config.terminalAgentCommand;
		this.terminalMaxUpdates = config.terminalMaxUpdates;
		this.terminalMaxUpdatesContinue = config.terminalMaxUpdatesContinue;
		this.terminalWinningScore = config.terminalWinningScore;
		this.terminalWinningScoreContinue = config.terminalWinningScoreContinue;
		this.terminalPassengerDelivered = config.terminalPassengerDelivered;
		this.terminalFuelRemaining = config.terminalFuelRemaining;
		this.terminalPassengerPickUp = config.terminalPassengerPickUp;
		
		// clients
		this.clients = new ArrayList<ClientConfig>(config.clients);
		
	}
	
	// loader fields
	public static final int kConfigVersion = 2;
	
	private static final String kTagSoar2D = "soar2d";
	private static final String kTagGeneral = "general";
	private static final String kTagLogging = "logging";
	private static final String kTagPlayers = "players";
	private static final String kTagTerminals = "terminals";
	private static final String kTagClients = "clients";
	
	private static final String kAttrVersion = "version";
	
	public void load(File configurationFile) throws LoadError {
		if (!configurationFile.exists()) {
			throw new LoadError("Config file doesn't exist: " + configurationFile.getAbsolutePath());
		}
		
		try {
			SAXBuilder builder = new SAXBuilder();
			Document doc = builder.build(configurationFile);
			Element root = doc.getRootElement();
			if (root == null || !root.getName().equalsIgnoreCase(kTagSoar2D)) {
				throw new LoadError("Couldn't find soar2d tag in configuration file.");
			}
			
			String versionString = root.getAttributeValue(kAttrVersion);
			if (versionString == null || Integer.parseInt(versionString) != kConfigVersion) {
				throw new LoadError("Configuration version 2 required, try removing the xml files from Soar2D.");
			}
			
			general(root.getChild(kTagGeneral));
			logging(root.getChild(kTagLogging));
			players(root.getChild(kTagPlayers));
			terminals(root.getChild(kTagTerminals));
			clients(root.getChild(kTagClients));
			
		} catch (IOException e) {
			throw new LoadError("I/O exception: " + e.getMessage());
		} catch (JDOMException e) {
			throw new LoadError("Error during parsing: " + e.getMessage());
		} catch (IllegalStateException e) {
			throw new LoadError("Illegal state: " + e.getMessage());
		}
	}
	
	public String generateXMLString() {
		Element root = new Element(kTagSoar2D);
		root.setAttribute(kAttrVersion, Integer.toString(kConfigVersion));
		
		Element general = new Element(kTagGeneral);
		generalSave(general);
		root.addContent(general);
		
		Element logging = new Element(kTagLogging);
		loggingSave(logging);
		root.addContent(logging);
		
		Element players = new Element(kTagPlayers);
		playersSave(players);
		root.addContent(players);

		Element terminals = new Element(kTagTerminals);
		terminalsSave(terminals);
		root.addContent(terminals);
		
		Element clients = new Element(kTagClients);
		clientsSave(clients);
		root.addContent(clients);

		XMLOutputter out = new XMLOutputter(Format.getPrettyFormat());
		return out.outputString(root);
	}
	
	// Type fields
	private static final String kTagGame = "game";
	private static final String kGameEaters = "eaters";
	private static final String kGameTankSoar = "tanksoar";
	private static final String kGameBook = "book";
	private static final String kGameKitchen = "kitchen";
	private static final String kGameTaxi = "taxi";
	private SimType simType = SimType.kEaters;
	public enum SimType { kEaters, kTankSoar, kBook, kKitchen, kTaxi }	
	private IConfiguration cModule;
	public void setType(SimType simType) {
		bConfig = null;
		eConfig = null;
		kConfig = null;
		tConfig = null;
		xConfig = null;
		this.simType = simType;
		switch (simType) {
		case kBook:
			cModule = new BookConfiguration();
			bConfig = (BookConfiguration)cModule.getModule();
			break;
		case kEaters:
			cModule = new EatersConfiguration(); 
			eConfig = (EatersConfiguration)cModule.getModule();
			break;
		case kKitchen:
			cModule = new KitchenConfiguration(); 
			kConfig = (KitchenConfiguration)cModule.getModule();
			break;
		case kTankSoar:
			cModule = new TankSoarConfiguration(); 
			tConfig = (TankSoarConfiguration)cModule.getModule();
			break;
		case kTaxi:
			cModule = new TaxiConfiguration(); 
			xConfig = (TaxiConfiguration)cModule.getModule();
			break;
		}
	}
	public SimType getType() {
		return this.simType;
	}
	public String getMapPath() {
		return cModule.getMapPath();
	}
	public String getAgentPath() {
		return cModule.getAgentPath();
	}

	public String getMapExt() {
		return cModule.getMapExt();
	}
	
	private static final String kTagMap = "map";
	public void setMap(File map) {
		cModule.setMap(map.getAbsoluteFile());
	}
	public File getMap() {
		return cModule.getMap();
	}
	
	private final int kDefaultPort = 12121;
	private int port = kDefaultPort; // use default
	private static final String kTagPort = "port";
	public void setPort(int port) {
		if (port < 0) {
			return;
		} else if (port > 65535) {
			return;
		}
		this.port = port;
	}
	public int getPort() {
		return this.port;
	}
	
	// random execution
	private boolean random = true; 	// If true, use random numbers (don't seed generators)
	private int randomSeed = 0;		// if random is false, this is the seed
	private static final String kTagSeed = "seed";
	public void setRandomSeed(int seed) {
		this.random = false;
		this.randomSeed = seed;
	}
	public void unsetRandomSeed() {
		this.random = true;
		this.randomSeed = 0;
	}
	public boolean hasRandomSeed() {
		return !this.random;
	}
	public int getRandomSeed() {
		return this.randomSeed;
	}
	
	// remote
	private boolean remote = false; // If true, connect to remote kernel
	private static final String kTagRemote = "remote";
	public void setRemote(boolean remote) {
		this.remote = remote;
	}
	public boolean getRemote() {
		return this.remote;
	}

	// hide
	private boolean hide = false; // If true, do not display the world map
	private static final String kTagHide = "hide";
	public void setHide(boolean hide) {
		this.hide = hide;
	}
	public boolean getHide() {
		return this.hide;
	}

	// gui
	private boolean nogui = false; // If true, do not use the window manager.
	private static final String kTagNoGUI = "no-gui";
	public void setNoGUI(boolean nogui) {
		this.nogui = nogui;
	}
	public boolean getNoGUI() {
		return this.nogui;
	}

	// async delay
	private int asyncTimeSlice = 0; // If positive, run soar asynchronously
	private static final String kTagASync = "async";
	public void setASyncDelay(int milliseconds) {
		this.asyncTimeSlice = milliseconds;
	}
	public int getASyncDelay() {
		return this.asyncTimeSlice;
	}
	
	// human input
	private boolean forceHumanInput = false; // override soar output with human output
	private static final String kTagForceHuman = "force-human";
	public void setForceHuman(boolean forceHumanInput) {
		this.forceHumanInput = forceHumanInput;
	}
	public boolean getForceHuman() {
		return this.forceHumanInput;
	}
	
	// metadata file
	private File metadata;
	private static final String kTagMetadata = "metadata";
	public void setMetadata(File metadata) {
		if (metadata == null) {
			this.metadata = null;
			return;
		}
		this.metadata = metadata.getAbsoluteFile();
	}
	public File getMetadata() {
		return this.metadata;
	}

	private void generalSave(Element general) {
		if (general == null) return;

		String gameText = cModule.getTitle().toLowerCase();
		general.addContent(new Element(kTagGame).setText(gameText));
		
		general.addContent(new Element(kTagMap).setText(cModule.getMap().getPath()));
		
		if (this.port != kDefaultPort) {
			general.addContent(new Element(kTagPort).setText(Integer.toString(this.port)));
		}
		
		if (this.hasRandomSeed()) {
			general.addContent(new Element(kTagSeed).setText(Integer.toString(this.getRandomSeed())));
		}
		
		if (this.getRemote()) {
			general.addContent(new Element(kTagRemote));
		}
		
		if (this.getHide()) {
			general.addContent(new Element(kTagHide));
		}
		
		if (this.getNoGUI()) {
			general.addContent(new Element(kTagNoGUI));
		}
		
		if (this.getSilentAgents()) {
			general.addContent(new Element(kTagSilentAgents));
		}
		
		if (this.getASyncDelay() > 0) {
			general.addContent(new Element(kTagASync).setText(Integer.toString(this.getASyncDelay())));
		}
		
		if (this.getForceHuman()) {
			general.addContent(new Element(kTagForceHuman));
		}
		
		if (this.getMetadata() != null) {
			general.addContent(new Element(kTagMetadata).setText(getMetadata().getPath()));
		}
		
		Element rules = new Element(kTagRules);
		cModule.rulesSave(rules);
		general.addContent(rules);
		
	}

	boolean toscaEnabled = false;
	private static final String kTagTosca = "tosca";
	public boolean getToscaEnabled() {
		return toscaEnabled;
	}
	public void setToscaEnabled(boolean setting) {
		this.toscaEnabled = setting;
	}
	
	private void general(Element general) throws LoadError {
		// this tag is required
		if (general == null) {
			throw new LoadError("General tag required.");
		}
		
		// get and remove required game tag and set game
		Element game = general.getChild(kTagGame);
		if (game == null) {
			throw new LoadError("Game tag required.");
		}
		general.removeChild(kTagGame);
		// set game
		String gameName = game.getTextTrim();
		if (gameName.equalsIgnoreCase(kGameEaters)) {
			this.setType(SimType.kEaters);
		} else if (gameName.equalsIgnoreCase(kGameTankSoar)) {
			this.setType(SimType.kTankSoar);
		} else if (gameName.equalsIgnoreCase(kGameBook)) {
			this.setType(SimType.kBook);
		} else if (gameName.equalsIgnoreCase(kGameKitchen)) {
			this.setType(SimType.kKitchen);
		} else if (gameName.equalsIgnoreCase(kGameTaxi)) {
			this.setType(SimType.kTaxi);
		} else {
			throw new LoadError("Unknown game type: " + gameName);
		}
		
		// get and remove required map tag and set map
		Element map = general.getChild(kTagMap);
		if (map == null) {
			throw new LoadError("Map tag required.");
		}
		general.removeChild(kTagMap);
		setMap(new File(map.getTextTrim()));
		
		// iterate through the rest of the children
		List<Element> children = (List<Element>)general.getChildren();
		Iterator<Element> iter = children.iterator();
		while (iter.hasNext()) {
			Element child = iter.next();
			
			if (child.getName().equalsIgnoreCase(kTagPort)) {
				try {
					setPort(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing port.");
				}
				
			} else if (child.getName().equalsIgnoreCase(kTagSeed)) {
				try {
					setRandomSeed(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing seed.");
				}
					
			} else if (child.getName().equalsIgnoreCase(kTagRemote)) {
				setRemote(true);
				
			} else if (child.getName().equalsIgnoreCase(kTagHide)) {
				setHide(true);
				
			} else if (child.getName().equalsIgnoreCase(kTagNoGUI)) {
				setNoGUI(true);
				
			} else if (child.getName().equalsIgnoreCase(kTagSilentAgents)) {
				setSilentAgents(true);
				
			} else if (child.getName().equalsIgnoreCase(kTagMaxMemoryUsage)) {
				try {
					setMaxMemoryUsage(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing max memory usage.");
				}
				
			} else if (child.getName().equalsIgnoreCase(kTagASync)) {
				try {
					setASyncDelay(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing async delay.");
				}

			} else if (child.getName().equalsIgnoreCase(kTagForceHuman)) {
				setForceHuman(true);

			} else if (child.getName().equalsIgnoreCase(kTagMetadata)) {
				setMetadata(new File(child.getTextTrim()));

			} else if (child.getName().equalsIgnoreCase(kTagRules)) {
				cModule.rules(child);

			} else if (child.getName().equalsIgnoreCase(kTagTosca)) {
				setToscaEnabled(true);

			} else {
				throw new LoadError("Unrecognized tag: " + child.getName());
			}
		}
	}
	
	// rules
	private static final String kTagRules = "rules";
	
	// log level
	private Level logLevel = Level.INFO; // Current log level, see java.util.logging
	private static final String kTagLevel = "level";
	public static final String[] kLogLevels = { "Severe", "Warning", "Info", "Fine", "Finer", "Finest" };
	public boolean setLogLevel(String level) {
		if (level.equalsIgnoreCase(kLogLevels[0])) {
			logLevel = Level.SEVERE;
		} else if (level.equalsIgnoreCase(kLogLevels[1])) {
			logLevel = Level.WARNING;
		} else if (level.equalsIgnoreCase(kLogLevels[2])) {
			logLevel = Level.INFO;
		} else if (level.equalsIgnoreCase(kLogLevels[3])) {
			logLevel = Level.FINE;
		} else if (level.equalsIgnoreCase(kLogLevels[4])) {
			logLevel = Level.FINER;
		} else if (level.equalsIgnoreCase(kLogLevels[5])) {
			logLevel = Level.FINEST;
		} else {
			return false;
		}
		return true;
	}
	public Level getLogLevel() {
		return this.logLevel;
	}
	
	// log file
	public static final File kDefaultLogFile = new File("soar2d.log");
	private static final String kTagFile = "file";
	private File logFile = kDefaultLogFile; // File to log to, only valid if logFile true
	public void setLogFile(File logFile) {
		this.logFile = logFile;
	}
	public File getLogFile() {
		return this.logFile;
	}
	
	// log console
	private boolean logConsole = false; // If true, log to console
	private static final String kTagConsole = "console";
	public void setLogConsole(boolean logConsole) {
		this.logConsole = logConsole;
	}
	public boolean getLogConsole() {
		return this.logConsole;
	}
	
	// log soar-print
	private boolean logSoarPrint = false; // If true, register for and log soar print events
	private static final String kTagSoarPrint = "soar-print";
	public void setLogSoarPrint(boolean logSoarPrint) {
		this.logSoarPrint = logSoarPrint;
	}
	public boolean getLogSoarPrint() {
		return this.logSoarPrint;
	}
	
	// log time
	private boolean logTime = false;	// if true, put a time stamp with each log entry
	private static final String kTagTime = "time";
	public void setLogTime(boolean logTime) {
		this.logTime = logTime;
	}
	public boolean getLogTime() {
		return this.logTime;
	}
	
	private void loggingSave(Element logging) {
		logging.addContent(new Element(kTagLevel).setText(logLevel.toString()));
		if (logFile != null) {
			logging.addContent(new Element(kTagFile).setText(logFile.getPath()));
		}
		
		if (this.getLogConsole()) {
			logging.addContent(new Element(kTagConsole));
		}
		
		if (this.getLogSoarPrint()) {
			logging.addContent(new Element(kTagSoarPrint));
		}
		
		if (this.getLogTime()) {
			logging.addContent(new Element(kTagTime));
		}
	}
	
	private void logging(Element logging) throws LoadError {
		if (logging == null) {
			return;
		}
		List<Element> children = (List<Element>)logging.getChildren();
		Iterator<Element> iter = children.iterator();
		while (iter.hasNext()) {
			Element child = iter.next();
			
			if (child.getName().equalsIgnoreCase(kTagLevel)) {
				if (!setLogLevel(child.getTextTrim())) {
					throw new LoadError("Invalid log level: " + child.getTextTrim());
				}
				
			} else if (child.getName().equalsIgnoreCase(kTagFile)) {
				setLogFile(new File(child.getTextTrim()));
				
			} else if (child.getName().equalsIgnoreCase(kTagConsole)) {
				setLogConsole(true);
				
			} else if (child.getName().equalsIgnoreCase(kTagSoarPrint)) {
				setLogSoarPrint(true);
				
			} else if (child.getName().equalsIgnoreCase(kTagTime)) {
				setLogTime(true);
				
			} else {
				throw new LoadError("Unrecognized tag: " + child.getName());
			}
		}
	}
	
	// debuggers
	private boolean debuggers = false; // If true, spawn debuggers on agent creation
	private static final String kTagDebuggers = "debuggers";
	public void setDebuggers(boolean debuggers) {
		this.debuggers = debuggers;
	}
	public boolean getDebuggers() {
		return this.debuggers;
	}
	
	// default points
	private int defaultPoints = 0;
	private static final String kTagDefaultPoints = "default-points";
	public void setDefaultPoints(int defaultPoints) {
		this.defaultPoints = defaultPoints;
	}
	public int getDefaultPoints() {
		return this.defaultPoints;
	}
	
	// players
	private static final String kTagPlayer = "player";
	private void playersSave(Element players) {
		if (this.getDebuggers()) {
			players.addContent(new Element(kTagDebuggers));
		}
		
		Iterator<PlayerConfig> iter = this.players.iterator();
		while (iter.hasNext()) {
			PlayerConfig config = iter.next();
			Element player = new Element(kTagPlayer);
			playerSave(player, config);
			players.addContent(player);
		}
		
	}
	
	private void players(Element players) throws LoadError {
		if (players == null) {
			return;
		}
		List<Element> children = (List<Element>)players.getChildren();
		Iterator<Element> iter = children.iterator();
		while (iter.hasNext()) {
			Element child = iter.next();
			
			if (child.getName().equalsIgnoreCase(kTagDebuggers)) {
				setDebuggers(true);
				
			} else if (child.getName().equalsIgnoreCase(kTagDefaultPoints)) {
				try {
					setDefaultPoints(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing default points");
				}

			} else if (child.getName().equalsIgnoreCase(kTagPlayer)) {
				player(child);

			} else {
				throw new LoadError("Unrecognized tag: " + child.getName());
			}
		}		
	}
	
	private static final String kTagName = "name";
	private static final String kTagProductions = "productions";
	private static final String kTagColor = "color";
	private static final String kTagX = "x";
	private static final String kTagY = "y";
	private static final String kTagFacing = "facing";
	private static final String kTagPoints = "points";
	private static final String kTagEnergy = "energy";
	private static final String kTagHealth = "health";
	private static final String kTagMissiles = "missiles";
	private static final String kTagShutdownCommand = "shutdown-command";
	private ArrayList<PlayerConfig> players = new ArrayList<PlayerConfig>(); // List of information required to create players
	public ArrayList<PlayerConfig> getPlayers() {
		return this.players;
	}
	
	private void playerSave(Element player, PlayerConfig config) {
		if (config.hasName()) {
			player.addContent(new Element(kTagName).setText(config.getName()));
		}
		
		if (config.hasProductions()) {
			player.addContent(new Element(kTagProductions).setText(config.getProductions().getPath()));
		}
		
		if (config.hasColor()) {
			player.addContent(new Element(kTagColor).setText(config.getColor()));
		}
		
		if (config.hasInitialLocation()) {
			player.addContent(new Element(kTagX).setText(Integer.toString(config.getInitialLocation().x)));
			player.addContent(new Element(kTagY).setText(Integer.toString(config.getInitialLocation().y)));
		}
		
		if (config.hasFacing()) {
			player.addContent(new Element(kTagFacing).setText(Direction.stringOf[config.getFacing()]));
		}
		
		if (config.hasPoints()) {
			player.addContent(new Element(kTagPoints).setText(Integer.toString(config.getPoints())));
		}
		
		if (config.hasEnergy()) {
			player.addContent(new Element(kTagEnergy).setText(Integer.toString(config.getEnergy())));
		}
		
		if (config.hasHealth()) {
			player.addContent(new Element(kTagHealth).setText(Integer.toString(config.getHealth())));
		}
		
		if (config.hasMissiles()) {
			player.addContent(new Element(kTagMissiles).setText(Integer.toString(config.getMissiles())));
		}
	}
	
	private void player(Element player) throws LoadError {
		List<Element> children = (List<Element>)player.getChildren();
		Iterator<Element> iter = children.iterator();
		
		PlayerConfig playerConfig = new PlayerConfig();
		java.awt.Point location = new java.awt.Point(-1, -1);
		
		while (iter.hasNext()) {
			Element child = iter.next();
			
			if (child.getName().equalsIgnoreCase(kTagName)) {
				playerConfig.setName(child.getTextTrim());
				
			} else if (child.getName().equalsIgnoreCase(kTagProductions)) {
				playerConfig.setProductions(new File(child.getTextTrim()));

			} else if (child.getName().equalsIgnoreCase(kTagColor)) {
				playerConfig.setColor(child.getTextTrim());

			} else if (child.getName().equalsIgnoreCase(kTagX)) {
				try {
					location.x = Integer.parseInt(child.getTextTrim());
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing x coordinate");
				}

			} else if (child.getName().equalsIgnoreCase(kTagY)) {
				try {
					location.y = Integer.parseInt(child.getTextTrim());
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing y coordinate");
				}

			} else if (child.getName().equalsIgnoreCase(kTagFacing)) {
				playerConfig.setFacing(Direction.getInt(child.getTextTrim()));

			} else if (child.getName().equalsIgnoreCase(kTagPoints)) {
				try {
					playerConfig.setPoints(Integer.parseInt(child.getTextTrim()));
					playerConfig.setPoints(true);
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing points");
				}


			} else if (child.getName().equalsIgnoreCase(kTagEnergy)) {
				try {
					playerConfig.setEnergy(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing energy");
				}


			} else if (child.getName().equalsIgnoreCase(kTagHealth)) {
				try {
					playerConfig.setHealth(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing health");
				}


			} else if (child.getName().equalsIgnoreCase(kTagMissiles)) {
				try {
					playerConfig.setMissiles(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing missiles");
				}
				
			} else if (child.getName().equalsIgnoreCase(kTagShutdownCommand)) {
				playerConfig.addShutdownCommand(child.getTextTrim());

			} else {
				throw new LoadError("Unrecognized tag: " + child.getName());
			}
		}		
		
		if (location.x != -1 || location.y != -1) {
			playerConfig.setInitialLocation(location);
		}
		
		players.add(playerConfig);
	}
	
	// terminals
	public void setDefaultTerminals() {
		clearAllTerminals();
		cModule.setDefaultTerminals(this);
	}
	
	private void clearAllTerminals() {
		this.terminalAgentCommand = false;
		this.terminalFoodRemaining = false;
		this.terminalFoodRemainingContinue = 0;
		this.terminalFuelRemaining = false;
		this.terminalMaxRuns = 0;
		this.terminalMaxUpdates = 0;
		this.terminalMaxUpdatesContinue = false;
		this.terminalPassengerDelivered = false;
		this.terminalPassengerPickUp = false;
		this.terminalPointsRemaining = false;
		this.terminalUnopenedBoxes = false;
		this.terminalWinningScore = 0;
		this.terminalWinningScoreContinue = false;
	}

	// terminal points remaining
	private boolean terminalPointsRemaining = false; // Stop the simulation when no points remain
	private static final String kTagPointsRemaining = "points-remaining";
	public void setTerminalPointsRemaining(boolean setting) {
		this.terminalPointsRemaining = setting;
	}
	public boolean getTerminalPointsRemaining() {
		return this.terminalPointsRemaining;
	}
	
	// terminal food remianing
	private boolean terminalFoodRemaining = false;	// Stop the simulation when no food remains
	private static final String kTagFoodRemaining = "food-remaining";
	public void setTerminalFoodRemaining(boolean setting) {
		this.terminalFoodRemaining = setting;
	}
	public boolean getTerminalFoodRemaining() {
		return this.terminalFoodRemaining;
	}
	// Reset and restart the simulation after this terminal is reached
	// int is the number of times to run before a forced stop
	// 0 or 1 is equivalent to not having this tag
	// negative values indicate run forever
	private int terminalFoodRemainingContinue = 0;	
	private static final String kTagFoodRemainingContinue = "continue";
	public void setTerminalFoodRemainingContinue(int runs) {
		this.terminalFoodRemainingContinue = runs;
	}
	public int getTerminalFoodRemainingContinue() {
		return this.terminalFoodRemainingContinue;
	}
	
	// terminal agent command
	private boolean terminalUnopenedBoxes = false;	// Stop the simulation when no boxes are closed
	private static final String kTagUnopenedBoxes = "unopened-boxes";
	public void setTerminalUnopenedBoxes(boolean setting) {
		this.terminalUnopenedBoxes = setting;
	}
	public boolean getTerminalUnopenedBoxes() {
		return this.terminalUnopenedBoxes;
	}
	
	// terminal winning score
	private boolean terminalAgentCommand = false;	// Stop the simulation when an agent issues a stop command.
	private static final String kTagAgentCommand = "agent-command";
	public void setTerminalAgentCommand(boolean setting) {
		this.terminalAgentCommand = setting;
	}
	public boolean getTerminalAgentCommand() {
		return this.terminalAgentCommand;
	}
	
	private int terminalMaxUpdates = 0;				// Stop the simulation when a certain number of world updates have completed.
	private static final String kTagMaxUpdates = "max-updates";
	public void setTerminalMaxUpdates(int maxUpdates) {
		this.terminalMaxUpdates = maxUpdates;
	}
	public int getTerminalMaxUpdates() {
		return this.terminalMaxUpdates;
	}
	private boolean terminalMaxUpdatesContinue = false;	
	private static final String kTagMaxUpdatesContinue = "continue";
	public void setTerminalMaxUpdatesContinue(boolean setting) {
		this.terminalMaxUpdatesContinue = setting;
	}
	public boolean getTerminalMaxUpdatesContinue() {
		return this.terminalMaxUpdatesContinue;
	}
	
	private int terminalWinningScore = 0;			// Stop the simulation when a certain number of world updates have completed.
	private static final String kTagWinningScore = "winning-score";
	public void setTerminalWinningScore(int winningScore) {
		this.terminalWinningScore = winningScore;
	}
	public int getTerminalWinningScore() {
		return this.terminalWinningScore;
	}
	private boolean terminalWinningScoreContinue = false;	
	private static final String kTagWinningScoreContinue = "continue";
	public void setTerminalWinningScoreContinue(boolean setting) {
		this.terminalWinningScoreContinue = setting;
	}
	public boolean getTerminalWinningScoreContinue() {
		return this.terminalWinningScoreContinue;
	}
	
	private int terminalMaxRuns = 0;				// 0 means no limit 
	private static final String kTagMaxRuns = "max-runs";
	public void setTerminalMaxRuns(int maxRuns) {
		this.terminalMaxRuns = maxRuns;
	}
	public int getTerminalMaxRuns() {
		return this.terminalMaxRuns;
	}
	
	private boolean terminalPassengerDelivered = false;	
	private static final String kTagPassengerDelivered = "passenger-delivered";
	public void setTerminalPassengerDelivered(boolean setting) {
		this.terminalPassengerDelivered = setting;
	}
	public boolean getTerminalPassengerDelivered() {
		return this.terminalPassengerDelivered;
	}
	
	private boolean terminalFuelRemaining = false;	
	private static final String kTagFuelRemaining = "fuel-remaining";
	public void setTerminalFuelRemaining(boolean setting) {
		this.terminalFuelRemaining = setting;
	}
	public boolean getTerminalFuelRemaining() {
		return this.terminalFuelRemaining;
	}
	
	private boolean terminalPassengerPickUp = false;	
	private static final String kTagPassengerPickUp = "passenger-pick-up";
	public void setTerminalPassengerPickUp(boolean setting) {
		this.terminalPassengerPickUp = setting;
	}
	public boolean getTerminalPassengerPickUp() {
		return this.terminalPassengerPickUp;
	}
	
	private void terminalsSave(Element terminals) {
		if (this.getTerminalMaxUpdates() > 0) {
			Element maxUpdates = new Element(kTagMaxUpdates).setText(Integer.toString(this.getTerminalMaxUpdates()));
			if (this.getTerminalMaxUpdatesContinue()) {
				maxUpdates.addContent(new Element(kTagMaxUpdatesContinue));
			}
			terminals.addContent(maxUpdates);
		}
		
		if (this.getTerminalAgentCommand()) {
			terminals.addContent(new Element(kTagAgentCommand));
		}
		
		if (this.getTerminalPointsRemaining()) {
			terminals.addContent(new Element(kTagPointsRemaining));
		}
		
		if (this.getTerminalWinningScore() > 0) {
			Element winningScore = new Element(kTagWinningScore).setText(Integer.toString(this.getTerminalWinningScore()));
			if (this.getTerminalWinningScoreContinue()) {
				winningScore.addContent(new Element(kTagWinningScoreContinue));
			}
			terminals.addContent(winningScore);
		}
		
		if (this.getTerminalFoodRemaining()) {
			if (this.getTerminalFoodRemainingContinue() > 0) {
				// <food-remaining>
				//   <continue>
				//     int maxRuns
				Element cont = new Element(kTagFoodRemainingContinue);
				cont.setText(Integer.toString(this.getTerminalFoodRemainingContinue()));
				Element foodRem = new Element(kTagFoodRemaining).addContent(cont);
				terminals.addContent(foodRem);
			} else {
				terminals.addContent(new Element(kTagFoodRemaining));
			}
		}
		
		if (this.getTerminalUnopenedBoxes()) {
			terminals.addContent(new Element(kTagUnopenedBoxes));
		}

		if (this.getTerminalMaxRuns() > 0) {
			terminals.addContent(new Element(kTagMaxRuns).setText(Integer.toString(this.terminalMaxRuns)));
		}
		
		if (this.getTerminalPassengerDelivered()) {
			terminals.addContent(new Element(kTagPassengerDelivered));
		}
		
		if (this.getTerminalPassengerDelivered()) {
			terminals.addContent(new Element(kTagFuelRemaining));
		}
		
		if (this.getTerminalPassengerPickUp()) {
			terminals.addContent(new Element(kTagPassengerPickUp));
		}
	}
	
	private void terminals(Element terminals) throws LoadError {
		if (terminals == null) {
			return;
		}
		List<Element> children = (List<Element>)terminals.getChildren();
		Iterator<Element> iter = children.iterator();
		
		while (iter.hasNext()) {
			Element child = iter.next();
			
			if (child.getName().equalsIgnoreCase(kTagMaxUpdates)) {
				try {
					this.setTerminalMaxUpdates(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing max updates");
				}
				Element cont = child.getChild(kTagMaxUpdatesContinue);
				if (cont != null) {
					this.setTerminalMaxUpdatesContinue(true);
				}

			} else if (child.getName().equalsIgnoreCase(kTagAgentCommand)) {
				this.setTerminalAgentCommand(true);
				
			} else if (child.getName().equalsIgnoreCase(kTagPointsRemaining)) {
				this.setTerminalPointsRemaining(true);
				
			} else if (child.getName().equalsIgnoreCase(kTagWinningScore)) {
				try {
					this.setTerminalWinningScore(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing winning score");
				}
				Element cont = child.getChild(kTagWinningScoreContinue);
				if (cont != null) {
					this.setTerminalWinningScoreContinue(true);
				}

			} else if (child.getName().equalsIgnoreCase(kTagFoodRemaining)) {
				this.setTerminalFoodRemaining(true);
				
				Element cont = child.getChild(kTagFoodRemainingContinue);
				if (cont != null) {
					try {
						this.setTerminalFoodRemainingContinue(Integer.parseInt(cont.getTextTrim()));
					} catch (NumberFormatException e) {
						throw new LoadError("Error parsing food-remaining continue max runs");
					}
				}
				
			} else if (child.getName().equalsIgnoreCase(kTagUnopenedBoxes)) {
				this.setTerminalUnopenedBoxes(true);
				
			} else if (child.getName().equalsIgnoreCase(kTagMaxRuns)) {
				try {
					this.setTerminalMaxRuns(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing max runs");
				}

			} else if (child.getName().equalsIgnoreCase(kTagPassengerDelivered)) {
				this.setTerminalPassengerDelivered(true);
				
			} else if (child.getName().equalsIgnoreCase(kTagFuelRemaining)) {
				this.setTerminalFuelRemaining(true);

			} else if (child.getName().equalsIgnoreCase(kTagPassengerPickUp)) {
				this.setTerminalPassengerPickUp(true);

			} else {
				throw new LoadError("Unrecognized tag: " + child.getName());
			}
		}
	}
	
	// clients
	private static final String kTagClient = "client";
	
	private void clientsSave(Element clients) {
		Iterator<ClientConfig> iter = this.clients.iterator();
		while (iter.hasNext()) {
			ClientConfig config = iter.next();
			Element client = new Element(kTagClient);
			clientSave(client, config);
			clients.addContent(client);
		}
		
	}
	
	private void clients(Element clients) throws LoadError {
		if (clients == null) {
			return;
		}
		List<Element> children = (List<Element>)clients.getChildren();
		Iterator<Element> iter = children.iterator();
		while (iter.hasNext()) {
			Element child = iter.next();
			
			if (child.getName().equalsIgnoreCase(kTagClient)) {
				client(child);
				
			} else {
				throw new LoadError("Unrecognized tag: " + child.getName());
			}
		}		
	}
	
	private static final String kTagClientName = "name";
	private static final String kTagCommand = "command";
	private static final String kTagTimeout = "timeout";
	private static final String kTagAfter = "after";
	public ArrayList<ClientConfig> clients = new ArrayList<ClientConfig>(); // List of clients that will connect to the simulation
	public ArrayList<ClientConfig> getClients() {
		return this.clients;
	}

	private void clientSave(Element client, ClientConfig config) {
		client.addContent(new Element(kTagName).setText(config.name));
		
		if (config.command != null) {
			client.addContent(new Element(kTagCommand).setText(config.command));
		}
		
		client.addContent(new Element(kTagCommand).setText(Integer.toString(config.timeout)));

		if (config.after) {
			client.addContent(new Element(kTagAfter));
		}
	}
	
	private void client(Element client) throws LoadError {
		List<Element> children = (List<Element>)client.getChildren();
		Iterator<Element> iter = children.iterator();
		
		ClientConfig clientConfig = new ClientConfig();
		
		while (iter.hasNext()) {
			Element child = iter.next();
			
			if (child.getName().equalsIgnoreCase(kTagClientName)) {
				clientConfig.name = child.getTextTrim();
				
			} else if (child.getName().equalsIgnoreCase(kTagCommand)) {
				clientConfig.command = child.getTextTrim();

			} else if (child.getName().equalsIgnoreCase(kTagTimeout)) {
				try {
					clientConfig.timeout = Integer.parseInt(child.getTextTrim());
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing timeout");
				}
				
			} else if (child.getName().equalsIgnoreCase(kTagAfter)) {
				clientConfig.after = true;

			} else {
				throw new LoadError("Unrecognized tag: " + child.getName());
			}
		}		
		
		clients.add(clientConfig);
	}
	
	public String getTitle() {
		if (cModule == null) {
			return "Soar2D";
		}
		return cModule.getTitle();
	}

	public boolean getRunTilOutput() {
		return cModule.getRunTilOutput();
	}

	public int getCycleTimeSlice() {
		return cModule.getCycleTimeSlice();
	}

	private boolean silentAgents = false;
	private String kTagSilentAgents = "silent-agents";
	public void setSilentAgents(boolean setting) {
		silentAgents = setting;
	}
	public boolean getSilentAgents() {
		return silentAgents;
	}
	
	private int maxMemoryUsageValue = 0;
	private String kTagMaxMemoryUsage = "max-memory-usage";
	public void setMaxMemoryUsage(int maxMemoryUsageValue) {
		this.maxMemoryUsageValue = maxMemoryUsageValue;
	}
	public int getMaxMemoryUsage() {
		return maxMemoryUsageValue;
	}
}
