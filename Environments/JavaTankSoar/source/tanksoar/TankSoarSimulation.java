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
	private static final String kTagAgents = "agents";
	private static final String kParamName = "name";
	private static final String kParamProductions = "productions";
	private static final String kParamColor = "color";
	private static final String kParamX = "x";
	private static final String kParamY = "y";
	private static final String kParamFacing = "facing";
	private static final String kDefaultMap = "default.tmap";
		
	private TankSoarWorld m_World;

	public TankSoarSimulation(String settingsFile, boolean quiet) {		
		super();
		
		// Log the settings file
		m_Logger.log("Settings file: " + settingsFile);

		String [] initialNames = null;
		String [] initialProductions = null;
		String [] initialColors = null;
		MapPoint [] initialLocations = null;
		String [] initialFacing = null;
	
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
					
					m_Logger.log("Default map: " + defaultMap);
					
				} else if (tagName.equalsIgnoreCase(kTagAgents)) {
					initialNames = new String[child.getNumberChildren()];
					initialProductions = new String[child.getNumberChildren()];
					initialColors = new String[child.getNumberChildren()];
					initialLocations = new MapPoint[child.getNumberChildren()];
					initialFacing = new String[child.getNumberChildren()];
					
					for (int j = 0; j < initialNames.length; ++j) {
						JavaElementXML agent = child.getChild(j);
						
						initialNames[j] = agent.getAttribute(kParamName);
						initialProductions[j] = agent.getAttribute(kParamProductions);
						
						// Next two lines kind of a hack.  Convert / to \\ on windows, and vice versa
						if (System.getProperty("file.separator").equalsIgnoreCase("\\")) {
							initialProductions[j] = initialProductions[j].replaceAll("/", "\\\\");
						} else if (System.getProperty("file.separator").equalsIgnoreCase("/")) {
							initialProductions[j] = initialProductions[j].replaceAll("\\\\", "/");
						}
						
						initialColors[j] = agent.getAttribute(kParamColor);
						initialLocations[j] = new MapPoint(agent.getAttributeIntDefault(kParamX, -1), agent.getAttributeIntDefault(kParamY, -1));
						initialFacing[j] = agent.getAttribute(kParamFacing);
						
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
		
		// add initial eaters
		if (initialNames != null) {
			for (int i = 0; i < initialNames.length; ++i) {
				createEntity(initialNames[i], getAgentPath() + initialProductions[i], initialColors[i], initialLocations[i], initialFacing[i]);
			}
		}
		
		// if in quiet mode, run!
		if (quiet) {
	    	startSimulation(false);
		}
	}
	
    public void createEntity(String name, String productions, String color, MapPoint location, String facing) {
    	if (name == null || productions == null) {
    		fireErrorMessage("Failed to create agent, name, productions or color null.");
    		return;
    	}
    	
    	if (location != null) {
    		if ((location.x == -1) || (location.y == -1)) {
    			location = null;
    		}
    	}
    	
		Agent agent = createAgent(name, productions);
		if (agent == null) {
			return;
		}
		m_World.createTank(agent, productions, color, location, facing);
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