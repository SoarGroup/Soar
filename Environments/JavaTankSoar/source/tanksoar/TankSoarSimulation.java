package tanksoar;

import java.util.logging.*;

import simulation.*;
import sml.*;
import utilities.*;

public class TankSoarSimulation extends Simulation implements SimulationManager {

	private static Logger logger = Logger.getLogger("simulation");
	
	public static final int kMaxTanks = 7;
	public static final String kMapFilter = "*.tmap";
	
	private static final String kTagTankSoar = "tanksoar";
	private static final String kTagSimulation = "simulation";
	private static final String kParamDebuggers = "debuggers";
	private static final String kParamDefaultMap = "default-map";
	private static final String kParamRuns = "runs";
	private static final String kParamMaxUpdates = "max-updates";
	private static final String kParamWinningScore = "winning-score";
	private static final String kTagAgents = "agents";
	private static final String kParamName = "name";
	private static final String kParamProductions = "productions";
	private static final String kParamColor = "color";
	private static final String kParamX = "x";
	private static final String kParamY = "y";
	private static final String kParamFacing = "facing";
	private static final String kParamEnergy = "energy";
	private static final String kParamHealth = "health";
	private static final String kParamMissiles = "missiles";
	private static final String kDefaultMap = "default.tmap";
		
	private TankSoarWorld m_World;
	private MoveInfo m_HumanInput;
	private int m_WinningScore;

	public TankSoarSimulation(String settingsFile, boolean quiet, boolean noRandom) {		
		super(noRandom, true);
		
		String [] initialNames = null;
		String [] initialProductions = null;
		String [] initialColors = null;
		java.awt.Point [] initialLocations = null;
		String [] initialFacing = null;
		int [] initialEnergy = null;
		int [] initialHealth = null;
		int [] initialMissiles = null;
	
		// Load settings file
		try {
			if (logger.isLoggable(Level.FINER)) logger.finer("Parsing settings file: " + settingsFile);
			JavaElementXML root = JavaElementXML.ReadFromFile(settingsFile);
			if (!root.getTagName().equalsIgnoreCase(kTagTankSoar)) {
				throw new Exception("Top level tag not " + kTagTankSoar);
			}
			// TODO: Version check
			
			for (int i = 0 ; i < root.getNumberChildren() ; ++i)
			{
				JavaElementXML child = root.getChild(i) ;
				String tagName = child.getTagName() ;
				
				if (tagName.equalsIgnoreCase(kTagSimulation)) {
					setSpawnDebuggers(child.getAttributeBooleanDefault(kParamDebuggers, true));
					String defaultMap = child.getAttribute(kParamDefaultMap);
					if (defaultMap == null) {
						defaultMap = kDefaultMap;
					}
					setDefaultMap(defaultMap);
					setRuns(child.getAttributeIntDefault(kParamRuns, 0));
					setMaxUpdates(child.getAttributeIntDefault(kParamMaxUpdates, 0));
					setWinningScore(child.getAttributeIntDefault(kParamWinningScore, 50));
										
					if (logger.isLoggable(Level.FINEST)) {
						logger.finest("Spawn debuggers: " + Boolean.toString(getSpawnDebuggers()));
						logger.finest("Default map: " + defaultMap);
						logger.finest("Runs: " + Integer.toString(getRuns()));
						logger.finest("Max updates: " + Integer.toString(getMaxUpdates()));
						logger.finest("Winning score: " + Integer.toString(getWinningScore()));
					}
					
				} else if (tagName.equalsIgnoreCase(kTagAgents)) {
					initialNames = new String[child.getNumberChildren()];
					initialProductions = new String[child.getNumberChildren()];
					initialColors = new String[child.getNumberChildren()];
					initialLocations = new java.awt.Point[child.getNumberChildren()];
					initialFacing = new String[child.getNumberChildren()];
					initialEnergy = new int[child.getNumberChildren()];
					initialHealth = new int[child.getNumberChildren()];
					initialMissiles = new int[child.getNumberChildren()];
					
					for (int j = 0; j < initialNames.length; ++j) {
						JavaElementXML agent = child.getChild(j);
						
						initialNames[j] = agent.getAttribute(kParamName);
						initialProductions[j] = agent.getAttribute(kParamProductions);
						
						if (initialProductions[j] != null) { 
							// Next two lines kind of a hack.  Convert / to \\ on windows, and vice versa
							if (System.getProperty("file.separator").equalsIgnoreCase("\\")) {
								initialProductions[j] = initialProductions[j].replaceAll("/", "\\\\");
							} else if (System.getProperty("file.separator").equalsIgnoreCase("/")) {
								initialProductions[j] = initialProductions[j].replaceAll("\\\\", "/");
							}
						}
						
						initialColors[j] = agent.getAttribute(kParamColor);
						initialLocations[j] = new java.awt.Point(agent.getAttributeIntDefault(kParamX, -1), agent.getAttributeIntDefault(kParamY, -1));
						initialFacing[j] = agent.getAttribute(kParamFacing);
						
						initialEnergy[j] = agent.getAttributeIntDefault(kParamEnergy, -1);
						initialHealth[j] = agent.getAttributeIntDefault(kParamHealth, -1);
						initialMissiles[j] = agent.getAttributeIntDefault(kParamMissiles, -1);
						
						if (logger.isLoggable(Level.FINEST)) {
							logger.finest("Name: " + initialNames[j]);
							logger.finest("   Productions: " + initialProductions[j]);
							logger.finest("   Color: " + initialColors[j]);
							logger.finest("   Location: " + initialLocations[j].toString());
							logger.finest("   Facing: " + initialFacing[j]);
							logger.finest("   Energy: " + Integer.toString(initialEnergy[j]));
							logger.finest("   Health: " + Integer.toString(initialHealth[j]));
							logger.finest("   Missiles: " + Integer.toString(initialMissiles[j]));
						}
					}
				} else {
					// Throw during development, but really we should just ignore this
					// when reading XML (in case we add tags later and load this into an earlier version)
					throw new Exception("Unknown tag " + tagName) ;
				}
			}				
		} catch (Exception e) {
			fireErrorMessageSevere("Error loading XML settings: " + e.getMessage());
			shutdown();
			System.exit(1);
		}

		setCurrentMap(getMapPath() + getDefaultMap());

		// Load default world
		m_World = new TankSoarWorld(this, quiet);
		setWorldManager(m_World);
		resetSimulation(false);
		
		// add initial tanks
		if (initialNames != null) {
			for (int i = 0; i < initialNames.length; ++i) {
				if (initialProductions[i] != null) {
					createEntity(initialNames[i], getAgentPath() + initialProductions[i], 
							initialColors[i], initialLocations[i], initialFacing[i],
							initialEnergy[i], initialHealth[i], initialMissiles[i]);
				} else {
					createEntity(initialNames[i], null, 
							initialColors[i], initialLocations[i], initialFacing[i],
							initialEnergy[i], initialHealth[i], initialMissiles[i]);
				}
			}
		}
		
		// if in quiet mode, run!
		if (quiet) {
			if (m_World.getTanks() == null || m_World.getTanks().length == 0) {
				super.fireErrorMessageSevere("Quiet mode started with no tanks!");
				shutdown();
				return;
			}
			if (m_World.getTanks().length == 1) {
				super.fireErrorMessageSevere("Quiet mode started with only one tank!");
				shutdown();
				return;
			}
			if (logger.isLoggable(Level.FINER)) logger.finer("Quiet mode execution starting.");
			startSimulation(false);
			shutdown();
		}
	}
	
	void notificationMessage(String notifyMessage) {
		super.fireNotificationMessage(notifyMessage);
	}
	
	public void setWinningScore(int score) {
		if (score <= 0) {
			logger.warning("Invalid winning-score: " + Integer.toString(score) + ", resetting to 50.");
			m_WinningScore = 50;
			return;
		}
		m_WinningScore = score;
	}
	
	public int getWinningScore() {
		return m_WinningScore;
	}
	
	void readHumanInput() {
		super.fireSimulationEvent(SimulationListener.kHumanInputEvent);
	}
	
	MoveInfo getHumanInput() {
		return m_HumanInput;
	}
	
	public void setHumanInput(MoveInfo humanInput) {
		m_HumanInput = humanInput;
	}
	
    public void createEntity(String name, String productionsIn, String color, java.awt.Point locationIn, String facing,
    		int energy, int health, int missiles) {
    	if (name == null || color == null) {
    		fireErrorMessageWarning("Failed to create agent, name or color null.");
    		return;
    	}
    	
    	java.awt.Point location = null;
    	if (locationIn != null) {
    		if ((locationIn.x == -1) || (locationIn.y == -1)) {
    			if (logger.isLoggable(Level.FINER)) logger.finer("Ignoring non-null location " + locationIn.toString());
    		} else {
    			location = new java.awt.Point(locationIn);
    		}
    	}
    	
    	Agent agent = null;
    	String productions = null;
    	if (productionsIn == null) {
    		// Human agent
    		if (logger.isLoggable(Level.FINER)) logger.finer("Creating human agent.");
    		productions = new String(name);
    	} else {
    		productions = new String(productionsIn);
			agent = createAgent(name, productions);
			if (agent == null) {
	    		fireErrorMessageWarning("Failed to create agent.");
				return;
			}
			if (logger.isLoggable(Level.FINER)) logger.finer("Created Soar agent.");
    	}
		m_World.createTank(agent, productions, color, location, facing, energy, health, missiles);
		spawnDebugger(name);		
		fireSimulationEvent(SimulationListener.kAgentCreatedEvent);   	
    }
        
	public TankSoarWorld getTankSoarWorld() {
		return m_World;
	}
	
	public WorldManager getWorldManager() {
		return getTankSoarWorld();
	}
	
    public World getWorld() {
		return getTankSoarWorld();
	}
	
	public void changeMap(String map) {
		setCurrentMap(map);
		resetSimulation(true);
	}

	public void destroyTank(Tank tank) {
		if (tank == null) {
    		logger.warning("Asked to destroy null agent, ignoring.");
    		return;
		}	
		m_World.destroyTank(tank);
		fireSimulationEvent(SimulationListener.kAgentDestroyedEvent);
	}
	
}
