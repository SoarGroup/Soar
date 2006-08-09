package tanksoar;

import java.util.logging.*;

import simulation.*;
import sml.*;

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
	private static final String kTagAgent = "agent";
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
	private static final String kFalse = "false";
		
	private TankSoarWorld m_World;
	private MoveInfo m_HumanInput;
	private int m_WinningScore = 50;

	public TankSoarSimulation(String settingsFile, boolean quiet, boolean noRandom) {		
		super(noRandom, true);
		
		String [] initialNames = new String[0];
		String [] initialProductions = new String[0];
		String [] initialColors = new String[0];
		java.awt.Point [] initialLocations = new java.awt.Point[0];
		String [] initialFacing = new String[0];
		int [] initialEnergy = new int[0];
		int [] initialHealth = new int[0];
		int [] initialMissiles = new int[0];
	
		setWinningScore(50);
		setDefaultMap(kDefaultMap);

		// Load settings file
		ElementXML rootTag = ElementXML.ParseXMLFromFile(settingsFile);
		if (rootTag == null) {
			fireErrorMessageSevere("Error parsing file: " + ElementXML.GetLastParseErrorDescription());
			shutdown();
			System.exit(1);
		}
		
		if (rootTag.IsTag(kTagTankSoar)) {
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
							
						} else if (attribute.equalsIgnoreCase(kParamWinningScore)) {
							setWinningScore(Integer.parseInt(value));
						}
					}
					
				} else if (mainTag.IsTag(kTagAgents)) {
					
					ElementXML agentTag = null;
					
					int children = mainTag.GetNumberChildren();
					initialNames = new String[children];
					initialProductions = new String[children];
					initialColors = new String[children];
					initialLocations = new java.awt.Point[children];
					initialFacing = new String[children];
					initialEnergy = new int[children];
					initialHealth = new int[children];
					initialMissiles = new int[children];

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

							initialEnergy[agentIndex] = -1;
							initialHealth[agentIndex] = -1;
							initialMissiles[agentIndex] = -1;

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
									
								} else if (attribute.equalsIgnoreCase(kParamFacing)) {
									initialFacing[agentIndex] = value;
									
								} else if (attribute.equalsIgnoreCase(kParamEnergy)) {
									initialEnergy[agentIndex] = Integer.parseInt(value);
									
								} else if (attribute.equalsIgnoreCase(kParamHealth)) {
									initialHealth[agentIndex] = Integer.parseInt(value);
									
								} else if (attribute.equalsIgnoreCase(kParamMissiles)) {
									initialMissiles[agentIndex] = Integer.parseInt(value);
									
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
		m_World = new TankSoarWorld(this, quiet);
		setWorldManager(m_World);
		resetSimulation(false);
		
		// add initial tanks
		for (int i = 0; i < initialNames.length; ++i) {
			if (initialNames[i] == null) {
				continue;
			}
			String productions = null;
			if (initialProductions[i] != null) {
				productions = getAgentPath() + initialProductions[i];
			}
			createEntity(initialNames[i], productions, initialColors[i], initialLocations[i], initialFacing[i],
					initialEnergy[i], initialHealth[i], initialMissiles[i]);
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
	
	public void errorMessageWarning(String message) {
		fireErrorMessageWarning(message);
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
