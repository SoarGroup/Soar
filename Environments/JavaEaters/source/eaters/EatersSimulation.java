package eaters;

import java.util.logging.*;

import simulation.*;
import sml.*;
import utilities.*;

public class EatersSimulation extends Simulation implements SimulationManager {
	private static Logger logger = Logger.getLogger("simulation");
	public static final int kMaxEaters = 7;	
	public static final String kMapFilter = "*.emap";
	
	private static final String kTagEaters = "eaters";
	private static final String kTagSimulation = "simulation";
	private static final String kParamDebuggers = "debuggers";
	private static final String kParamDefaultMap = "default-map";
	private static final String kParamRuns = "runs";
	private static final String kParamMaxUpdates = "max-updates";
	private static final String kTagAgents = "agents";
	private static final String kParamName = "name";
	private static final String kParamProductions = "productions";
	private static final String kParamColor = "color";
	private static final String kDefaultMap = "default.emap";
	private static final String kParamX = "x";
	private static final String kParamY = "y";

	private EatersWorld m_EatersWorld;

	public EatersSimulation(String settingsFile, boolean quiet, boolean notRandom) {	
		super(notRandom, false);
		
		// Log the settings file
		logger.fine("Settings file: " + settingsFile);

		String [] initialNames = null;
		String [] initialProductions = null;
		String [] initialColors = null;
		MapPoint [] initialLocations = null;
		
		// Load settings file
		try {
			JavaElementXML root = JavaElementXML.ReadFromFile(settingsFile);
			if (!root.getTagName().equalsIgnoreCase(kTagEaters)) {
				throw new Exception("Top level tag not " + kTagEaters);
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
					
					logger.fine("Default map: " + defaultMap);
					
				} else if (tagName.equalsIgnoreCase(kTagAgents)) {
					initialNames = new String[child.getNumberChildren()];
					initialProductions = new String[child.getNumberChildren()];
					initialColors = new String[child.getNumberChildren()];
					initialLocations = new MapPoint[child.getNumberChildren()];

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
		m_EatersWorld = new EatersWorld(this);
		setWorldManager(m_EatersWorld);
		resetSimulation(false);
		
		// add initial eaters
		if (initialNames != null) {
			for (int i = 0; i < initialNames.length; ++i) {
				createEntity(initialNames[i], getAgentPath() + initialProductions[i], initialColors[i], initialLocations[i], null, -1, -1, -1);
			}
		}
		
		// if in quiet mode, run!
		if (quiet) {
	    	startSimulation(false);
	    	shutdown();
		}
	}
	
    public void createEntity(String name, String productions, String color, MapPoint location, String facing, int energy, int health, int missiles) {
    	if (facing != null || energy != -1 || health != -1 || missiles != -1) {
    		logger.warning("An ignored parameter was given a non-default value!");
    	}
    	
    	if (location != null) {
    		if ((location.x == -1) || (location.y == -1)) {
    			location = null;
    		}
    	}
    	
    	if (name == null || productions == null) {
    		fireErrorMessageWarning("Failed to create agent, name, productions or color null.");
    		return;
    	}
    	
		Agent agent = createAgent(name, productions);
		if (agent == null) {
			return;
		}
		m_EatersWorld.createEater(agent, productions, color, location);
		spawnDebugger(name);		
		fireSimulationEvent(SimulationListener.kAgentCreatedEvent);   	
    }
        
    public EatersWorld getEatersWorld() {
    	return m_EatersWorld;
    }
    
    public WorldManager getWorldManager() {
		return getEatersWorld();
	}
	
    public World getWorld() {
		return getEatersWorld();
	}
}