package soar2d;

import java.util.*;
import java.util.logging.*;
import java.io.*;

import org.jdom.Document;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;
import org.jdom.Element;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

import soar2d.player.*;

/**
 * @author voigtjr
 * 
 * This class holds all the configuration for the current simulation.
 * Not all fields are applicable for all types of simulations.
 */
public class Configuration {
	
	// misc non-configurable configuration

	public Configuration() {}
	
	public Configuration(Configuration config) {
		
		// meta
		this.setType(config.simType);
		this.map = config.map.getAbsoluteFile();
		this.port = config.port;

		// general
		this.random = config.random;
		this.randomSeed = config.randomSeed;
		this.remote = config.remote;
		this.hide = config.hide;
		this.nogui = config.nogui;
		this.asyncTimeSlice = config.asyncTimeSlice;

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

		// book-specific
		this.coloredRooms = config.coloredRooms;
		this.speed = config.speed;
		this.bookCellSize = config.bookCellSize;
		this.cycleTimeSlice = config.cycleTimeSlice;
		
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
	
	public class LoadError extends Exception {
		static final long serialVersionUID = 0;
		
		public LoadError(String message) {
			super(message);
		}
	}
	
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
	private String basePath = null; // Path to Soar2D folder
	private String mapPath = null; // Path to maps
	private String agentPath = null; // Path to agents
	private SimType simType = SimType.kEaters;
	public enum SimType { kEaters, kTankSoar, kBook }	
	public void setType(SimType simType) {
		this.simType = simType;
		this.generatePaths();
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
	private final String kDefaultEatersMap = "random-walls.emap";
	private final String kDefaultTanksoarMap = "default.tmap";
	private final String kDefaultBookMap = "default.bmap";
	public final String kMapDir = "maps";
	public final String kAgentDir = "agents";
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
	public static final String kBookMapExt = "bmap";
	public static String kEatersMapExt = "emap";
	public static String kTankSoarMapExt = "tmap";
	public String getMapExt() {
		switch(simType) {
		case kEaters:
			return kEatersMapExt;
		case kTankSoar:
			return kTankSoarMapExt;
		case kBook:
			return kBookMapExt;
		}
		assert false;
		return kEatersMapExt;
	}
	
	private File map = null; // The current map file
	private static final String kTagMap = "map";
	public void setMap(File map) {
		this.map = map.getAbsoluteFile();
	}
	public File getMap() {
		return this.map;
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
	
	private void generalSave(Element general) {
		if (general == null) return;

		String gameText = new String();
		switch (this.simType) {
		case kEaters:
			gameText = kTagEaters;
			break;
		case kTankSoar:
			gameText = kTagTankSoar;
			break;
		case kBook:
			gameText = kTagBook;
			break;
		}
		general.addContent(new Element(kTagGame).setText(gameText));
		
		general.addContent(new Element(kTagMap).setText(this.map.getPath()));
		
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
		
		if (this.getASyncDelay() > 0) {
			general.addContent(new Element(kTagASync).setText(Integer.toString(this.getASyncDelay())));
		}
		
		Element rules = new Element(kTagRules);
		rulesSave(rules);
		general.addContent(rules);
		
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
		List children = general.getChildren();
		Iterator iter = children.iterator();
		while (iter.hasNext()) {
			Element child = (Element)iter.next();
			
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
				
			} else if (child.getName().equalsIgnoreCase(kTagASync)) {
				try {
					setASyncDelay(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing async delay.");
				}

			} else if (child.getName().equalsIgnoreCase(kTagRules)) {
				rules(child);

			} else {
				throw new LoadError("Unrecognized tag: " + child.getName());
			}
		}
	}
	
	// rules
	private static final String kTagRules = "rules";
	
	private void rulesSave(Element rules) {
		switch (this.simType) {
		case kEaters:
			Element eaters = new Element(kTagEaters);
			eaters.addContent(new Element(kTagVision).setText(Integer.toString(this.getEaterVision())));
			eaters.addContent(new Element(kTagWallPenalty).setText(Integer.toString(this.getWallPenalty())));
			eaters.addContent(new Element(kTagJumpPenalty).setText(Integer.toString(this.getJumpPenalty())));
			eaters.addContent(new Element(kTagLowProbability).setText(Double.toString(this.getLowProbability())));
			eaters.addContent(new Element(kTagHighProbability).setText(Double.toString(this.getHighProbability())));
			rules.addContent(eaters);
			break;
		case kTankSoar:
			Element tanksoar = new Element(kTagTankSoar);
			tanksoar.addContent(new Element(kTagDefaultMissiles).setText(Integer.toString(this.getDefaultMissiles())));
			tanksoar.addContent(new Element(kTagDefaultEnergy).setText(Integer.toString(this.getDefaultEnergy())));
			tanksoar.addContent(new Element(kTagDefaultHealth).setText(Integer.toString(this.getDefaultHealth())));
			tanksoar.addContent(new Element(kTagCollisionPenalty).setText(Integer.toString(this.getCollisionPenalty())));
			tanksoar.addContent(new Element(kTagMaxMissilePacks).setText(Integer.toString(this.getMaxMissilePacks())));
			tanksoar.addContent(new Element(kTagMissilePackRespawnChance).setText(Integer.toString(this.getMissilePackRespawnChance())));
			tanksoar.addContent(new Element(kTagShieldEnergyUsage).setText(Integer.toString(this.getShieldEnergyUsage())));
			tanksoar.addContent(new Element(kTagMissileHitAward).setText(Integer.toString(this.getMissileHitAward())));
			tanksoar.addContent(new Element(kTagMissileHitPenalty).setText(Integer.toString(this.getMissileHitPenalty())));
			tanksoar.addContent(new Element(kTagKillAward).setText(Integer.toString(this.getKillAward())));
			tanksoar.addContent(new Element(kTagKillPenalty).setText(Integer.toString(this.getKillPenalty())));
			tanksoar.addContent(new Element(kTagRadarWidth).setText(Integer.toString(this.getRadarWidth())));
			tanksoar.addContent(new Element(kTagRadarHeight).setText(Integer.toString(this.getRadarHeight())));
			tanksoar.addContent(new Element(kTagMaxSmellDistance).setText(Integer.toString(this.getMaxSmellDistance())));
			tanksoar.addContent(new Element(kTagMissileResetThreshold).setText(Integer.toString(this.getMissileResetThreshold())));
			rules.addContent(tanksoar);
			break;
		case kBook:
			Element book = new Element(kTagBook);
			if (this.getColoredRooms()) {
				book.addContent(new Element(kTagColoredRooms));
			}
			book.addContent(new Element(kTagSpeed).setText(Integer.toString(this.getSpeed())));
			book.addContent(new Element(kTagRotateSpeed).setText(Float.toString(this.getRotateSpeed())));
			book.addContent(new Element(kTagBookCellSize).setText(Integer.toString(this.getBookCellSize())));
			book.addContent(new Element(kTagCycleTimeSlice).setText(Integer.toString(this.getCycleTimeSlice())));
			rules.addContent(book);
			break;
		}
	}
	
	private void rules(Element rules) throws LoadError {
		List children = rules.getChildren();
		Iterator iter = children.iterator();
		while (iter.hasNext()) {
			Element child = (Element)iter.next();
			
			if (child.getName().equalsIgnoreCase(kTagEaters)) {
				eaters(child);
				
			} else if (child.getName().equalsIgnoreCase(kTagTankSoar)) {
				tanksoar(child);

			} else if (child.getName().equalsIgnoreCase(kTagBook)) {
				book(child);

			} else {
				throw new LoadError("Unrecognized tag: " + child.getName());
			}
		}		
	}
	
	private static final String kTagEaters = "eaters";
	
	private static final String kTagVision = "vision";
	private int eaterVision = 2;	// distance that eater can see
	public void setEaterVision(int distance) {
		this.eaterVision = distance;
	}
	public int getEaterVision() {
		return this.eaterVision;
	}
	
	private static final String kTagWallPenalty = "wall-penalty";
	private int wallPenalty = -5;	// eaters scoring penalty
	public void setWallPenalty(int points) {
		this.wallPenalty = points;
	}
	public int getWallPenalty() {
		return this.wallPenalty;
	}
	
	private static final String kTagJumpPenalty = "jump-penalty";
	private int jumpPenalty = -5;	// eaters scoring penalty
	public void setJumpPenalty(int points) {
		this.jumpPenalty = points;
	}
	public int getJumpPenalty() {
		return this.jumpPenalty;
	}
	
	private static final String kTagLowProbability = "low-probability";
	private double lowProbability = .15;		// eaters wall generation
	public void setLowProbability(double prob) {
		this.lowProbability = prob;
	}
	public double getLowProbability() {
		return this.lowProbability;
	}
	
	private static final String kTagHighProbability = "high-probability";
	private double higherProbability = .65;	// eaters wall generation
	public void setHighProbability(double prob) {
		this.higherProbability = prob;
	}
	public double getHighProbability() {
		return this.higherProbability;
	}

	private void eaters(Element eaters) throws LoadError {
		List children = eaters.getChildren();
		Iterator iter = children.iterator();
		while (iter.hasNext()) {
			Element child = (Element)iter.next();
			
			if (child.getName().equalsIgnoreCase(kTagVision)) {
				try {
					this.setEaterVision(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagVision);
				}
				
			} else if (child.getName().equalsIgnoreCase(kTagWallPenalty)) {
				try {
					this.setWallPenalty(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagWallPenalty);
				}

			} else if (child.getName().equalsIgnoreCase(kTagJumpPenalty)) {
				try {
					this.setJumpPenalty(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagJumpPenalty);
				}

			} else if (child.getName().equalsIgnoreCase(kTagLowProbability)) {
				try {
					this.setLowProbability(Double.parseDouble(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagLowProbability);
				}

			} else if (child.getName().equalsIgnoreCase(kTagHighProbability)) {
				try {
					this.setHighProbability(Double.parseDouble(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagHighProbability);
				}

			} else {
				throw new LoadError("Unrecognized tag: " + child.getName());
			}
		}		
	}
	
	private static final String kTagTankSoar = "tanksoar";

	private static final String kTagDefaultMissiles = "default-missiles";
	private int defaultMissiles = 15;
	public void setDefaultMissiles(int setting) {
		this.defaultMissiles = setting;
	}
	public int getDefaultMissiles() {
		return this.defaultMissiles;
	}
	
	private static final String kTagDefaultEnergy = "default-energy";
	private int defaultEnergy = 1000;
	public void setDefaultEnergy(int setting) {
		this.defaultEnergy = setting;
	}
	public int getDefaultEnergy() {
		return this.defaultEnergy;
	}
	
	private static final String kTagDefaultHealth = "default-health";
	private int defaultHealth = 1000;
	public void setDefaultHealth(int setting) {
		this.defaultHealth = setting;
	}
	public int getDefaultHealth() {
		return this.defaultHealth;
	}
	
	private static final String kTagCollisionPenalty = "collision-penalty";
	private int tankCollisionPenalty = -100;	// tanksoar
	public void setCollisionPenalty(int setting) {
		this.tankCollisionPenalty = setting;
	}
	public int getCollisionPenalty() {
		return this.tankCollisionPenalty;
	}
	
	private static final String kTagMaxMissilePacks = "max-missile-packs";
	private int maxMissilePacks = 3;
	public void setMaxMissilePacks(int setting) {
		this.maxMissilePacks = setting;
	}
	public int getMaxMissilePacks() {
		return this.maxMissilePacks;
	}
	
	private static final String kTagMissilePackRespawnChance = "missile-pack-respawn-chance";
	private int missilePackRespawnChance = 5;	// percent chance per update that a missile pack will respawn
	public void setMissilePackRespawnChance(int setting) {
		this.missilePackRespawnChance = setting;
	}
	public int getMissilePackRespawnChance() {
		return this.missilePackRespawnChance;
	}
	
	private static final String kTagShieldEnergyUsage = "shield-energy-usage";
	private int sheildEnergyUsage = -20;
	public void setShieldEnergyUsage(int setting) {
		this.sheildEnergyUsage = setting;
	}
	public int getShieldEnergyUsage() {
		return this.sheildEnergyUsage;
	}
	
	private static final String kTagMissileHitAward = "missile-hit-award";
	private int missileHitAward = 2;
	public void setMissileHitAward(int setting) {
		this.missileHitAward = setting;
	}
	public int getMissileHitAward() {
		return this.missileHitAward;
	}
	
	private static final String kTagMissileHitPenalty = "missile-hit-penalty";
	private int missileHitPenalty = -1;
	public void setMissileHitPenalty(int setting) {
		this.missileHitPenalty = setting;
	}
	public int getMissileHitPenalty() {
		return this.missileHitPenalty;
	}
	
	private static final String kTagKillAward = "kill-award";
	private int killAward = 3;
	public void setKillAward(int setting) {
		this.killAward = setting;
	}
	public int getKillAward() {
		return this.killAward;
	}
	
	private static final String kTagKillPenalty = "kill-penalty";
	private int killPenalty = -2;
	public void setKillPenalty(int setting) {
		this.killPenalty = setting;
	}
	public int getKillPenalty() {
		return this.killPenalty;
	}
	
	private static final String kTagRadarWidth = "radar-width";
	private int radarWidth = 3;
	public void setRadarWidth(int setting) {
		this.radarWidth = setting;
	}
	public int getRadarWidth() {
		return this.radarWidth;
	}
	
	private static final String kTagRadarHeight = "radar-height";
	private int radarHeight = 15;
	public void setRadarHeight(int setting) {
		this.radarHeight = setting;
	}
	public int getRadarHeight() {
		return this.radarHeight;
	}
	
	private static final String kTagMaxSmellDistance = "max-smell-distance";
	private int maxSmellDistance = 7;
	public void setMaxSmellDistance(int setting) {
		this.maxSmellDistance = setting;
	}
	public int getMaxSmellDistance() {
		return this.maxSmellDistance;
	}
	
	private static final String kTagMissileResetThreshold = "missile-reset-threshold";
	private int missileResetThreshold = 100;
	public void setMissileResetThreshold(int setting) {
		this.missileResetThreshold = setting;
	}
	public int getMissileResetThreshold() {
		return this.missileResetThreshold;
	}
	
	private void tanksoar(Element tanksoar) throws LoadError {
		List children = tanksoar.getChildren();
		Iterator iter = children.iterator();
		while (iter.hasNext()) {
			Element child = (Element)iter.next();
			
			if (child.getName().equalsIgnoreCase(kTagDefaultMissiles)) {
				try {
					this.setDefaultMissiles(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagDefaultMissiles);
				}
				
			} else if (child.getName().equalsIgnoreCase(kTagDefaultEnergy)) {
				try {
					this.setDefaultEnergy(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagDefaultEnergy);
				}

			} else if (child.getName().equalsIgnoreCase(kTagDefaultHealth)) {
				try {
					this.setDefaultHealth(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagDefaultHealth);
				}

			} else if (child.getName().equalsIgnoreCase(kTagCollisionPenalty)) {
				try {
					this.setCollisionPenalty(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagCollisionPenalty);
				}

			} else if (child.getName().equalsIgnoreCase(kTagMaxMissilePacks)) {
				try {
					this.setMaxMissilePacks(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagMaxMissilePacks);
				}

			} else if (child.getName().equalsIgnoreCase(kTagMissilePackRespawnChance)) {
				try {
					this.setMissilePackRespawnChance(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagMissilePackRespawnChance);
				}

			} else if (child.getName().equalsIgnoreCase(kTagShieldEnergyUsage)) {
				try {
					this.setShieldEnergyUsage(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagShieldEnergyUsage);
				}

			} else if (child.getName().equalsIgnoreCase(kTagMissileHitAward)) {
				try {
					this.setMissileHitAward(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagMissileHitAward);
				}

			} else if (child.getName().equalsIgnoreCase(kTagMissileHitPenalty)) {
				try {
					this.setMissileHitPenalty(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagMissileHitPenalty);
				}

			} else if (child.getName().equalsIgnoreCase(kTagKillAward)) {
				try {
					this.setKillAward(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagKillAward);
				}

			} else if (child.getName().equalsIgnoreCase(kTagKillPenalty)) {
				try {
					this.setKillPenalty(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagKillPenalty);
				}

			} else if (child.getName().equalsIgnoreCase(kTagRadarWidth)) {
				try {
					this.setRadarWidth(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagRadarWidth);
				}

			} else if (child.getName().equalsIgnoreCase(kTagRadarHeight)) {
				try {
					this.setRadarHeight(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagRadarHeight);
				}

			} else if (child.getName().equalsIgnoreCase(kTagMaxSmellDistance)) {
				try {
					this.setMaxSmellDistance(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagMaxSmellDistance);
				}

			} else if (child.getName().equalsIgnoreCase(kTagMissileResetThreshold)) {
				try {
					this.setMissileResetThreshold(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagMissileResetThreshold);
				}

			} else {
				throw new LoadError("Unrecognized tag: " + child.getName());
			}
		}		
	}

	private static final String kTagBook = "book";
	
	private static final String kTagColoredRooms = "colored-rooms";
	private boolean coloredRooms = false;
	public boolean getColoredRooms() {
		return this.coloredRooms;
	}
	public void setColoredRooms(boolean coloredRooms) {
		this.coloredRooms = coloredRooms;
	}
	
	private static final String kTagSpeed = "speed";
	private int speed = 16;
	public int getSpeed() {
		return this.speed;
	}
	public void setSpeed(int speed) {
		this.speed = speed;
	}
	
	private static final String kTagRotateSpeed = "rotate-speed";
	private float rotateSpeed = (float)java.lang.Math.PI / 4;
	public float getRotateSpeed() {
		return this.rotateSpeed;
	}
	public void setRotateSpeed(float rotateSpeed) {
		this.rotateSpeed = rotateSpeed;
	}
	
	private static final String kTagBookCellSize = "book-cell-size";
	private int bookCellSize = 16;
	public int getBookCellSize() {
		return this.bookCellSize;
	}
	public void setBookCellSize(int bookCellSize) {
		this.bookCellSize = bookCellSize;
	}
	
	private static final String kTagCycleTimeSlice = "cycle-time-slice";
	private int cycleTimeSlice = 50;
	public int getCycleTimeSlice() {
		return this.cycleTimeSlice;
	}
	public void setCycleTimeSlice(int cycleTimeSlice) {
		this.cycleTimeSlice = cycleTimeSlice;
	}
	
	private void book(Element book) throws LoadError {
		List children = book.getChildren();
		Iterator iter = children.iterator();
		while (iter.hasNext()) {
			Element child = (Element)iter.next();
			
			if (child.getName().equalsIgnoreCase(kTagColoredRooms)) {
				this.setColoredRooms(true);
				
			} else if (child.getName().equalsIgnoreCase(kTagSpeed)) {
				try {
					this.setSpeed(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagSpeed);
				}
				
			} else if (child.getName().equalsIgnoreCase(kTagRotateSpeed)) {
				try {
					this.setRotateSpeed(Float.parseFloat(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagRotateSpeed);
				}
				
			} else if (child.getName().equalsIgnoreCase(kTagBookCellSize)) {
				try {
					this.setBookCellSize(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagBookCellSize);
				}
				
			} else if (child.getName().equalsIgnoreCase(kTagCycleTimeSlice)) {
				try {
					this.setCycleTimeSlice(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagCycleTimeSlice);
				}
				
			} else {
				throw new LoadError("Unrecognized tag: " + child.getName());
			}
		}		
	}

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
		List children = logging.getChildren();
		Iterator iter = children.iterator();
		while (iter.hasNext()) {
			Element child = (Element)iter.next();
			
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
		List children = players.getChildren();
		Iterator iter = children.iterator();
		while (iter.hasNext()) {
			Element child = (Element)iter.next();
			
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
		List children = player.getChildren();
		Iterator iter = children.iterator();
		
		PlayerConfig playerConfig = new PlayerConfig();
		java.awt.Point location = new java.awt.Point(-1, -1);
		
		while (iter.hasNext()) {
			Element child = (Element)iter.next();
			
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
			this.terminalWinningScore = 0;
			this.terminalFoodRemaining = false;
			break;
		}
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
	}
	
	private void terminals(Element terminals) throws LoadError {
		if (terminals == null) {
			return;
		}
		List children = terminals.getChildren();
		Iterator iter = children.iterator();
		
		while (iter.hasNext()) {
			Element child = (Element)iter.next();
			
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
		List children = clients.getChildren();
		Iterator iter = children.iterator();
		while (iter.hasNext()) {
			Element child = (Element)iter.next();
			
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
		List children = client.getChildren();
		Iterator iter = children.iterator();
		
		ClientConfig clientConfig = new ClientConfig();
		
		while (iter.hasNext()) {
			Element child = (Element)iter.next();
			
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
	
	// Unit test! No way!
	public static void main(String[] args) {
		// Print the default config file to the screen.
		Configuration config = new Configuration();
		config.setType(SimType.kEaters);
		System.out.println("Eaters:");
		System.out.println(config.generateXMLString());
		
		System.out.println();
		
		config = new Configuration();
		config.setType(SimType.kTankSoar);
		System.out.println("TankSoar:");
		System.out.println(config.generateXMLString());
	}
}
