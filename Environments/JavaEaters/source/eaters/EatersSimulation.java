package eaters;

import java.util.logging.*;

import simulation.*;
import sml.*;

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
	private static final String kTagAgent = "agent";
	private static final String kParamName = "name";
	private static final String kParamProductions = "productions";
	private static final String kParamProductionsAbsolute = "productions-absolute";
	private static final String kParamColor = "color";
	private static final String kDefaultMap = "random-walls.emap";
	private static final String kParamX = "x";
	private static final String kParamY = "y";
	
	private static final String kFalse = "false";

	private EatersWorld m_EatersWorld;

	public EatersSimulation(String settingsFile, boolean quiet, boolean notRandom) {	
		super(notRandom, false);
		
		// Log the settings file
		if (logger.isLoggable(Level.FINE)) logger.fine("Settings file: " + settingsFile);

		String [] initialNames = new String[0];
		String [] initialProductions = new String[0];
		String [] initialColors = new String[0];
		java.awt.Point [] initialLocations = new java.awt.Point[0];
		
		setDefaultMap(kDefaultMap);
		
		// Load settings file
		ElementXML rootTag = ElementXML.ParseXMLFromFile(settingsFile);
		if (rootTag == null) {
			fireErrorMessageSevere("Error parsing file: " + ElementXML.GetLastParseErrorDescription());
			shutdown();
			System.exit(1);
		}
		
		if (rootTag.IsTag(kTagEaters)) {
			ElementXML mainTag = null;
			for (int rootTagIndex = 0 ; rootTagIndex < rootTag.GetNumberChildren() ; ++rootTagIndex) {
				mainTag = new ElementXML();
				rootTag.GetChild(mainTag, rootTagIndex);
				if (mainTag == null) {
					assert false;
					continue;
				}
				
				if (mainTag.IsTag(kTagSimulation)) {
					for (int attrIndex = 0; attrIndex < mainTag.GetNumberAttributes(); ++attrIndex) {
						String attribute = mainTag.GetAttributeName(attrIndex);
						if (attribute == null) {
							assert false;
							continue;
						}
						
						String value = mainTag.GetAttributeValue(attrIndex);
						if (value == null) {
							assert false;
							continue;
						}
						
						if (attribute.equalsIgnoreCase(kParamDebuggers)) {
							if (value.equalsIgnoreCase(kFalse)) {
								setSpawnDebuggers(false);
							}
						
						} else if (attribute.equalsIgnoreCase(kParamDefaultMap)) {
							setDefaultMap(value);
							if (logger.isLoggable(Level.FINE)) logger.fine("Default map: " + value);
						
						} else if (attribute.equalsIgnoreCase(kParamRuns)) {
							setRuns(Integer.parseInt(value));

						} else if (attribute.equalsIgnoreCase(kParamMaxUpdates)) {
							setMaxUpdates(Integer.parseInt(value));
						}
					}
					
				} else if (mainTag.IsTag(kTagAgents)) {
					
					ElementXML agentTag = null;
					int children = mainTag.GetNumberChildren();
					initialNames = new String[children];
					initialProductions = new String[children];
					initialColors = new String[children];
					initialLocations = new java.awt.Point[children];

					for (int agentIndex = 0 ; agentIndex < children; ++agentIndex) {
						agentTag = new ElementXML();
						mainTag.GetChild(agentTag, agentIndex);
						if (agentTag == null) {
							assert false;
							continue;
						}
													
						if (agentTag.IsTag(kTagAgent)) {
							int x = -1;
							int y = -1;

							for (int attrIndex = 0; attrIndex < agentTag.GetNumberAttributes(); ++attrIndex) {
								String attribute = agentTag.GetAttributeName(attrIndex);
								if (attribute == null) {
									assert false;
									continue;
								}
								
								String value = agentTag.GetAttributeValue(attrIndex);
								if (value == null) {
									assert false;
									continue;
								}
								
								if (attribute.equalsIgnoreCase(kParamName)) {
									initialNames[agentIndex] = value;
									
								} else if (attribute.equalsIgnoreCase(kParamProductions)) {
									initialProductions[agentIndex] = getAgentPath() + value;
									// Next two lines kind of a hack.  Convert / to \\ on windows, and vice versa
									if (System.getProperty("file.separator").equalsIgnoreCase("\\")) {
										initialProductions[agentIndex] = initialProductions[agentIndex].replaceAll("/", "\\\\");
									} else if (System.getProperty("file.separator").equalsIgnoreCase("/")) {
										initialProductions[agentIndex] = initialProductions[agentIndex].replaceAll("\\\\", "/");
									}
									
								} else if (attribute.equalsIgnoreCase(kParamProductionsAbsolute)) {
									initialProductions[agentIndex] = value;
									// Next two lines kind of a hack.  Convert / to \\ on windows, and vice versa
									if (System.getProperty("file.separator").equalsIgnoreCase("\\")) {
										initialProductions[agentIndex] = initialProductions[agentIndex].replaceAll("/", "\\\\");
									} else if (System.getProperty("file.separator").equalsIgnoreCase("/")) {
										initialProductions[agentIndex] = initialProductions[agentIndex].replaceAll("\\\\", "/");
									}
									
								} else if (attribute.equalsIgnoreCase(kParamColor)) {
									initialColors[agentIndex] = value;
									
								} else if (attribute.equalsIgnoreCase(kParamX)) {
									x = Integer.parseInt(value);
									
								} else if (attribute.equalsIgnoreCase(kParamY)) {
									y = Integer.parseInt(value);
									
								}
							}			
							if (initialNames[agentIndex] == null) {
								logger.warning("Required name attribute missing, ignoring agent");
								continue;
							}
							initialLocations[agentIndex] = new java.awt.Point(x, y);
						} else {
							logger.warning("Unknown tag: " + agentTag.GetTagName());
						}
						agentTag.delete();
						agentTag = null;
					}
				} else {
					logger.warning("Unknown tag: " + mainTag.GetTagName());
				}
				mainTag.delete();
				mainTag = null;
			}
		} else {
			logger.warning("Unknown tag: " + rootTag.GetTagName());
			fireErrorMessageWarning("No root eaters tag.");
		}		
		rootTag.ReleaseRefOnHandle();
		rootTag = null;

		setCurrentMap(getMapPath() + getDefaultMap());

		// Load default world
		m_EatersWorld = new EatersWorld(this);
		setWorldManager(m_EatersWorld);
		resetSimulation(false);
		
		// add initial eaters
		for (int i = 0; i < initialNames.length; ++i) {
			if (initialNames[i] == null) {
				continue;
			}
			createEntity(initialNames[i], initialProductions[i], initialColors[i], initialLocations[i], null, -1, -1, -1);
		}
		
		// if in quiet mode, run!
		if (quiet) {
	    	startSimulation(false);
	    	shutdown();
		}
	}
	
	public void errorMessageWarning(String message) {
		fireErrorMessageWarning(message);
	}
	
    public void createEntity(String name, String productions, String color, java.awt.Point location, String facing, int energy, int health, int missiles) {
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