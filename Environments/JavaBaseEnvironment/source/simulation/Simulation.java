package simulation;

import java.util.*;

import sml.*;
import utilities.*;

public abstract class Simulation implements Runnable, Kernel.UpdateEventInterface, Kernel.SystemEventInterface {
	public static final String kAgentFolder = "agents";
	public static final String kMapFolder = "maps";
	public static final int kDebuggerTimeoutSeconds = 15;	
	
	protected Logger m_Logger = Logger.logger;
	
	private String m_CurrentMap;
	private String m_DefaultMap;
	private boolean m_Debuggers;	
	private Kernel m_Kernel;
	private boolean m_StopSoar = false;
	private int m_WorldCount = 0;
	private int m_Runs = 0;
	private int m_MaxUpdates = 0;
	private Thread m_RunThread;
	private boolean m_Running = false;
	private String m_LastErrorMessage = "No error.";
	private String m_BasePath;
	private WorldManager m_WorldManager;
	private ArrayList m_SimulationListeners = new ArrayList();
	private ArrayList m_AddSimulationListeners = new ArrayList();
	private ArrayList m_RemoveSimulationListeners = new ArrayList();

	// For debugging can set this to false, making all random calls follow the same sequence
	public static final boolean kRandom = true ;
	
	protected Simulation() {
		// Initialize Soar
		// Create kernel
		try {
			m_Kernel = Kernel.CreateKernelInNewThread("SoarKernelSML", 12121);
		} catch (Exception e) {
			fireErrorMessage("Exception while creating kernel: " + e.getMessage());
			System.exit(1);
		}

		if (m_Kernel.HadError()) {
			fireErrorMessage("Error creating kernel: " + m_Kernel.GetLastErrorDescription());
			System.exit(1);
		}
		
		// We want the most performance
		m_Kernel.SetAutoCommit(false);

		// Make all runs non-random if asked
		if (!kRandom)
			m_Kernel.ExecuteCommandLine("srand 0", null) ;
		
		// Register for events
		m_Kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, this, null);
		m_Kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, this, null);
		m_Kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_GENERATED_OUTPUT, this, null);
		
		// Generate base path
		m_BasePath = System.getProperty("user.dir") + System.getProperty("file.separator");
		m_Logger.log("Base path: " + m_BasePath);
	}
	
	protected void setWorldManager(WorldManager worldManager) {
		m_WorldManager = worldManager;
	}
	
	public String getLastErrorMessage() {
		return m_LastErrorMessage;
	}
	
	public int getRuns() {
		return m_Runs;
	}
	
  	public int getWorldCount() {
  		return m_WorldCount;
  	}
  	
  	public boolean reachedMaxUpdates() {
  		return m_MaxUpdates >= 0 ? (m_WorldCount >= m_MaxUpdates): false;
  	}
  	
	public void setCurrentMap(String map) {
		m_CurrentMap = map;
	}
	
	public String getCurrentMap() {
		return m_CurrentMap;
	}
	
	public void setDefaultMap(String map) {
		m_DefaultMap = map;
	}
	
	public String getDefaultMap() {
		return m_DefaultMap;
	}
	
	public String getAgentPath() {
		return m_BasePath + kAgentFolder + System.getProperty("file.separator");
	}
	
	public String getMapPath() {
		return m_BasePath + kMapFolder + System.getProperty("file.separator");
	}
	
    protected Agent createAgent(String name, String productions) {
    	Agent agent = m_Kernel.CreateAgent(name);
    	boolean load = agent.LoadProductions(productions);
    	if (!load || agent.HadError()) {
			fireErrorMessage("Failed to create agent " + name + " (" + productions + "): " + agent.GetLastErrorDescription());
			m_Kernel.DestroyAgent(agent);
			agent.delete();
    		return null;
    	}
    	return agent;
    }
        
	public void destroyEntity(WorldEntity entity) {
		if (entity == null) {
    		m_Logger.log("Asked to destroy null entity, ignoring.");
    		return;
		}	
		m_WorldManager.destroyEntity(entity);
		m_Kernel.DestroyAgent(entity.getAgent());
		fireSimulationEvent(SimulationListener.kAgentDestroyedEvent);
	}
	
	public void setRuns(int runs) {
		if (runs < 0) {
			runs = -1;
		}
		m_Runs = runs;
	}
	
	public void setMaxUpdates(int updates) {
		if (updates <= 0) {
			updates = -1;
		}
		m_MaxUpdates = updates;
	}
	
	public void changeMap(String map) {
		setCurrentMap(map);
		resetSimulation(true);
	}	

	protected void destroyAgent(Agent agent) {
		m_Kernel.DestroyAgent(agent);
		agent.delete();
    }
        
	public void setSpawnDebuggers(boolean mode) {
		m_Debuggers = mode;
	}
	
	public boolean getSpawnDebuggers() {
		return m_Debuggers;
	}
	
	public void spawnDebugger(String agentName) {
		if (!m_Debuggers) return;
		if (debuggerConnected()) return;
		
		// Figure out whether to use java or javaw
		String os = System.getProperty("os.name");
		String javabin = "java";
		if (os.matches(".+indows.*") || os.matches("INDOWS")) {
			javabin = "javaw";
		}
		
		Runtime r = java.lang.Runtime.getRuntime();
		String commandLine = javabin + " -jar \"" + m_BasePath
		+ ".." + System.getProperty("file.separator")
		+ ".." + System.getProperty("file.separator")
		+ "SoarLibrary" + System.getProperty("file.separator")
		+ "bin" + System.getProperty("file.separator") 
		+ "SoarJavaDebugger.jar\" -remote -agent " + agentName;
		try {
			// TODO: manage the returned process a bit better.
			r.exec(commandLine);
			
			if (!waitForDebugger()) {
				fireErrorMessage("Debugger spawn failed for agent: " + agentName);
				return;
			}
			
		} catch (java.io.IOException e) {
			fireErrorMessage("IOException spawning debugger: " + e.getMessage());
			shutdown();
			System.exit(1);
		}
	}
	
	private boolean waitForDebugger() {
		boolean ready = false;
		for (int tries = 0; tries < kDebuggerTimeoutSeconds; ++tries) {
			m_Kernel.GetAllConnectionInfo();
			if (m_Kernel.HasConnectionInfoChanged()) {
				for (int i = 0; i < m_Kernel.GetNumberConnections(); ++i) {
					ConnectionInfo info =  m_Kernel.GetConnectionInfo(i);
					if (info.GetName().equalsIgnoreCase("java-debugger")) {
						if (info.GetAgentStatus().equalsIgnoreCase(sml_Names.getKStatusReady())) {
							ready = true;
							break;
						}
					}
				}
				if (ready) break;
			}
			try { Thread.sleep(1000); } catch (InterruptedException ignored) {}
		}
		return ready;
	}
	
	private boolean debuggerConnected() {
		boolean connected = false;
		m_Kernel.GetAllConnectionInfo();
		for (int i = 0; i < m_Kernel.GetNumberConnections(); ++i) {
			ConnectionInfo info =  m_Kernel.GetConnectionInfo(i);
			if (info.GetName().equalsIgnoreCase("java-debugger")) {
				connected = true;
				break;
			}
		}
		return connected;
	}
	
	public void shutdown() {
		m_Logger.log("Shutdown called.");
		if (m_WorldManager != null) {
			m_WorldManager.shutdown();
		}
		if (m_Kernel != null) {
			m_Kernel.Shutdown();
			m_Kernel.delete();
		}
	}
	
	public void startSimulation(boolean inNewThread) {
        m_StopSoar = false;
		if (inNewThread) {
			m_RunThread = new Thread(this);
	        m_RunThread.start();
		} else {
			run();
		}
	}
	
	public void stepSimulation() {
		m_Kernel.RunAllTilOutput();
	}
	
	public void stopSimulation() {
		if (m_Runs == 0) {
			m_RunThread = null;
		}
		m_StopSoar = true;
	}
	
	public boolean isRunning() {
		return m_Running;
	}

	public void resetSimulation(boolean fallBackToDefault) {
		String fatalErrorMessage = null;
		if (!m_WorldManager.load(m_CurrentMap)) {
			if (fallBackToDefault) {
				fireErrorMessage("Error loading map, check log for more information. Loading default map.");
				// Fall back to default map
				m_CurrentMap = getMapPath() + m_DefaultMap;
				if (!m_WorldManager.load(m_CurrentMap)) {
					fatalErrorMessage = "Error loading default map, closing Eaters.";
				}
			} else {
				fatalErrorMessage = "Error loading map, check log for more information. Loading default map.";
			}
		}
		if (fatalErrorMessage != null) {
			fireErrorMessage(fatalErrorMessage);
			shutdown();
			System.exit(1);
		}
		m_WorldCount = 0;
		fireSimulationEvent(SimulationListener.kResetEvent);
	}

    public void run() {
    	do {
    		if (m_Runs > 0) {
    			--m_Runs;
    		}
    		
    		m_StopSoar = false;
    		m_Kernel.RunAllAgentsForever();
    		
    		if (m_Runs != 0) {
    			resetSimulation(false);
    		}
    	} while (m_Runs != 0);
    }
    
  	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) {
  		if (m_StopSoar) {
  			m_StopSoar = false;
  			m_Kernel.StopAllAgents();
  		}
  		//m_Logger.log("Update number " + m_WorldCount);
  		m_WorldManager.update();
		++m_WorldCount;
		fireSimulationEvent(SimulationListener.kUpdateEvent);
  	}
  	
    public void systemEventHandler(int eventID, Object data, Kernel kernel) {
  		if (eventID == smlSystemEventId.smlEVENT_SYSTEM_START.swigValue()) {
  			// Start simulation
  			m_Logger.log("Start event received from kernel.");
  			m_Running = true;
  			fireSimulationEvent(SimulationListener.kStartEvent);
  		} else if (eventID == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue()) {
  			// Stop simulation
  			m_Logger.log("Stop event received from kernel.");
  			m_Running = false;
  			if (m_Runs == 0) {
  				m_RunThread = null;
  			}
  			fireSimulationEvent(SimulationListener.kStopEvent);	
 		} else {
 			m_Logger.log("Unknown system event received from kernel: " + eventID);
 		}
    }
    
	protected void fireErrorMessage(String errorMessage) {
		m_LastErrorMessage = errorMessage;
		fireSimulationEvent(SimulationListener.kErrorMessageEvent);
		m_Logger.log(errorMessage);
	}
	
	public void addSimulationListener(SimulationListener listener) {
		m_AddSimulationListeners.add(listener);
	}
	
	public void removeSimulationListener(SimulationListener listener) {
		m_RemoveSimulationListeners.add(listener);
	}
	
	protected void fireSimulationEvent(int type) {
		updateSimulationListenerList();
		Iterator iter = m_SimulationListeners.iterator();
		while(iter.hasNext()){
			((SimulationListener)iter.next()).simulationEventHandler(type);
		}		
	}
		
	protected void updateSimulationListenerList() {
		Iterator iter = m_RemoveSimulationListeners.iterator();
		while(iter.hasNext()){
			m_SimulationListeners.remove(iter.next());
		}
		m_RemoveSimulationListeners.clear();
		
		iter = m_AddSimulationListeners.iterator();
		while(iter.hasNext()){
			m_SimulationListeners.add(iter.next());
		}
		m_AddSimulationListeners.clear();		
	}
}