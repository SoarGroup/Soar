package tanksoar;

import simulation.*;
import sml.*;
import utilities.*;

public class TankSoarSimulation extends Simulation implements SimulationManager {
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
		
		// Log the settings file
		m_Logger.log("Settings file: " + settingsFile);

		String [] initialNames = null;
		String [] initialProductions = null;
		String [] initialColors = null;
		MapPoint [] initialLocations = null;
		String [] initialFacing = null;
		int [] initialEnergy = null;
		int [] initialHealth = null;
		int [] initialMissiles = null;
	
		// Load settings file
		try {
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
										
					m_Logger.log("Default map: " + defaultMap);
					
				} else if (tagName.equalsIgnoreCase(kTagAgents)) {
					initialNames = new String[child.getNumberChildren()];
					initialProductions = new String[child.getNumberChildren()];
					initialColors = new String[child.getNumberChildren()];
					initialLocations = new MapPoint[child.getNumberChildren()];
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
						initialLocations[j] = new MapPoint(agent.getAttributeIntDefault(kParamX, -1), agent.getAttributeIntDefault(kParamY, -1));
						initialFacing[j] = agent.getAttribute(kParamFacing);
						
						initialEnergy[j] = agent.getAttributeIntDefault(kParamEnergy, -1);
						initialHealth[j] = agent.getAttributeIntDefault(kParamHealth, -1);
						initialMissiles[j] = agent.getAttributeIntDefault(kParamMissiles, -1);
						
					}
				} else {
					// Throw during development, but really we should just ignore this
					// when reading XML (in case we add tags later and load this into an earlier version)
					throw new Exception("Unknown tag " + tagName) ;
				}
			}				
		} catch (Exception e) {
			fireErrorMessage("Error loading XML settings: " + e.getMessage());
			shutdown();
			System.exit(1);

		}

		setCurrentMap(getMapPath() + getDefaultMap());

		// Load default world
		m_World = new TankSoarWorld(this);
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
			if (m_World.getTanks() == null) {
				super.fireErrorMessage("Quiet mode started with no tanks!");
				shutdown();
				return;
			}
			startSimulation(false);
			shutdown();
		}
	}
	
	void notificationMessage(String notifyMessage) {
		super.fireNotificationMessage(notifyMessage);
	}
	
	public void setWinningScore(int score) {
		if (score <= 0) {
			m_Logger.log("Invalid winning-score parameter in config file, resetting to 50.");
			score = 50;
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
	
    public void createEntity(String name, String productions, String color, MapPoint location, String facing,
    		int energy, int health, int missiles) {
    	if (name == null || color == null) {
    		fireErrorMessage("Failed to create agent, name or color null.");
    		return;
    	}
    	
    	if (location != null) {
    		if ((location.x == -1) || (location.y == -1)) {
    			location = null;
    		}
    	}
    	
    	Agent agent = null;
    	if (productions == null) {
    		// Human agent
    		productions = name;
    	} else {
			agent = createAgent(name, productions);
			if (agent == null) {
	    		fireErrorMessage("Failed to create agent.");
				return;
			}
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
    		m_Logger.log("Asked to destroy null agent, ignoring.");
    		return;
		}	
		m_World.destroyTank(tank);
		fireSimulationEvent(SimulationListener.kAgentDestroyedEvent);
	}
	
}
