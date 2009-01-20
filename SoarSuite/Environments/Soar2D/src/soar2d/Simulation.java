package soar2d;

import java.io.*;
import java.util.*;
import java.util.Map.Entry;

import org.apache.log4j.Logger;
import sml.*;
import soar2d.config.ClientConfig;
import soar2d.config.PlayerConfig;
import soar2d.config.SimConfig;
import soar2d.player.*;
import soar2d.player.book.Dog;
import soar2d.player.book.Mouse;
import soar2d.player.book.Robot;
import soar2d.player.book.SoarRobot;
import soar2d.player.eaters.Eater;
import soar2d.player.eaters.SoarEater;
import soar2d.player.eaters.ToscaEater;
import soar2d.player.kitchen.Cook;
import soar2d.player.kitchen.SoarCook;
import soar2d.player.kitchen.ToscaCook;
import soar2d.player.tanksoar.SoarTank;
import soar2d.player.tanksoar.Tank;
import soar2d.player.taxi.SoarTaxi;
import soar2d.player.taxi.Taxi;
import soar2d.world.World;

/**
 * @author voigtjr
 *
 * Keeps track of the meta simulation state. The world keeps track of more state and
 * is the major member of this class. Creates the soar kernel and registers events.
 */
public class Simulation {
	private static Logger logger = Logger.getLogger(Simulation.class);

	/**
	 * True if we want to use the run-til-output feature
	 */
	boolean runTilOutput = false;
	/**
	 * The soar kernel
	 */
	Kernel kernel = null;
	/**
	 * The random number generator used throughout the program
	 */
	public static Random random = null;
	
	/**
	 * The world and everything associated with it
	 */
	public World world = new World();
	/**
	 * Legal colors (see PlayerConfig)
	 */
	public final String kColors[] = { "red", "blue", "yellow", "purple", "orange", "green", "black",  };
	/**
	 * A list of colors not currently taken by a player
	 */
	private ArrayList<String> unusedColors = new ArrayList<String>(kColors.length);
	/**
	 * String agent name to agent mapping
	 */
	private HashMap<String, Agent> agents = new HashMap<String, Agent>();

	/**
	 * @return true if there were no errors during initialization
	 * 
	 * sets everything up in preparation of execution. only called once per
	 * program run (not once per soar run)
	 */
	public boolean initialize() {
		// keep track of colors
		for (String color : kColors) {
			unusedColors.add(color);
		}
		
		runTilOutput = Soar2D.config.runTilOutput();
		
		// Initialize Soar
		if (Soar2D.config.soarConfig().remote != null) {
			kernel = Kernel.CreateRemoteConnection(true, Soar2D.config.soarConfig().remote, Soar2D.config.soarConfig().port);
		} else {
			// Create kernel
			kernel = Kernel.CreateKernelInNewThread("SoarKernelSML", Soar2D.config.soarConfig().port);
			//kernel = Kernel.CreateKernelInCurrentThread("SoarKernelSML", true);
		}

		if (kernel.HadError()) {
			fatalError(Names.Errors.kernelCreation + kernel.GetLastErrorDescription());
			return false;
		}
		
		// We want the most performance
		logger.debug(Names.Debug.autoCommit);
		kernel.SetAutoCommit(false);

		// Make all runs non-random if asked
		// For debugging, set this to make all random calls follow the same sequence
		if (Soar2D.config.hasSeed()) {
			// seed the generators
			int seed = Soar2D.config.generalConfig().seed;
			logger.debug(Names.Debug.seed + seed);
			kernel.ExecuteCommandLine("srand " + seed, null) ;
			random = new Random(seed);
		} else {
			logger.debug(Names.Debug.noSeed);
			random = new Random();
		}
		
		// Register for events
		logger.trace(Names.Trace.eventRegistration);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, Soar2D.control, null);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, Soar2D.control, null);
		if (runTilOutput) {
			logger.debug(Names.Debug.runTilOutput);
			kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_GENERATED_OUTPUT, Soar2D.control, null);
		} else {
			logger.debug(Names.Debug.noRunTilOutput);
			kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, Soar2D.control, null);
		}
		
		// Load the world
		logger.trace(Names.Trace.loadingWorld);
		if(!world.load()) {
			return false;
		}
		
		// Start or wait for clients (false: before agent creation)
		logger.trace(Names.Trace.beforeClients);
		if (!doClients(false)) {
			return false;
		}
		
		// add initial players
		logger.trace(Names.Trace.initialPlayers);
		for ( Entry<String, PlayerConfig> entry : Soar2D.config.playerConfigs().entrySet()) {
			createPlayer(entry.getKey(), entry.getValue());
		}
		
		// Start or wait for clients (true: after agent creation)
		logger.trace(Names.Trace.afterClients);
		if (!doClients(true)) {
			return false;
		}
		
		// success
		return true;
	}
	
	private void fatalError(String message) {
		logger.fatal(message);
		Soar2D.control.errorPopUp(message);
	}
	
	private void error(String message) {
		logger.fatal(message);
		Soar2D.control.errorPopUp(message);
	}
	
	public ArrayList<String> getUnusedColors() {
		return unusedColors;
	}
	
	/**
	 * @param color the color to use, or null if any will work
	 * @return null if the color is not available for whatever reason
	 * 
	 * removes a color from the unused colors list (by random if necessary)
	 * a return of null indicates failure, the color is taken or no more
	 * are available
	 */
	public String useAColor(String color) {
		if (unusedColors.size() < 1) {
			return null;
		}
		if (color == null) {
			int pick = random.nextInt(unusedColors.size());
			color = unusedColors.get(pick);
			unusedColors.remove(pick);
			return color;
		}
		Iterator<String> iter = unusedColors.iterator();
		while (iter.hasNext()) {
			if (color.equalsIgnoreCase(iter.next())) {
				iter.remove();
				return color;
			}
		}
		return null;
	}
	
	/**
	 * @param color the color to free up, must be not null
	 * @return false if the color wasn't freed up
	 * 
	 * The opposite of useAColor
	 * a color wouldn't be freed up if it wasn't being used in the first place
	 * or if it wasn't legal
	 */
	public boolean freeAColor(String color) {
		assert color != null;
		boolean legal = false;
		for (String knownColor : kColors) {
			if (color.equals(knownColor)) {
				legal = true;
			}
		}
		if (!legal) {
			return false;
		}
		if (unusedColors.contains(color)) {
			return false;
		}
		unusedColors.add(color);
		return true;
	}

	/**
	 * @author voigtjr
	 *
	 * exception class to keep things sane during player creation
	 */
	class CreationException extends Throwable {
		static final long serialVersionUID = 1;
		private String message;
		
		public CreationException() {
		}
		
		public CreationException(String message) {
			this.message = message;
		}
		
		public String getMessage() {
			return message;
		}
	}

	/**
	 * @param playerConfig configuration data for the future player
	 * 
	 * create a player and add it to the simulation and world
	 */
	public void createPlayer(String playerId, PlayerConfig playerConfig) {
		if (SimConfig.Game.TAXI == Soar2D.config.game() && (world.getPlayers().size() > 1)) {
			// if this is removed, revisit white color below!
			error(Names.Errors.taxi1Player);
			return;
		}
		
		// if a color was specified
		if (playerConfig.color != null) {
			//make sure it is unused
			if (!unusedColors.contains(playerConfig.color)) {
				error(Names.Errors.usedColor + playerConfig.color);
				return;
			}
			// it is unused, so use it
			useAColor(playerConfig.color);
		} else {
			
			// no color specified, pick on at random
			String color = useAColor(null);
			
			// make sure we got one
			if (color == null) {
				
				// if we didn't then they are all gone
				error(Names.Errors.noMoreSlots);
				return;
			}
			playerConfig.color = color;
		}
		
		// if we don't have a name, use our color
		if (playerConfig.name == null) {
			playerConfig.name = playerConfig.color;
		}
		
		try {
			// check for duplicate name
			if (world.getPlayers().get(playerConfig.name) != null) {
				throw new CreationException("Failed to create player: " + playerConfig.name + " already exists.");
			}
			
			// check for human agent
			if (playerConfig.productions == null) {
				
				// create a human agent
				Player player = null;
				
				// eater or tank depending on the setting
				boolean human = true;
				switch(Soar2D.config.game()) {
				case EATERS:
					if (Soar2D.config.generalConfig().tosca) {
						player = new ToscaEater(playerId);
						human = false;
					} else {
						player = new Eater(playerId);
					}
					break;
					
				case TANKSOAR:
					player = new Tank(playerId);
					break;
					
				case ROOM:
					if (playerConfig.name.equals(Names.kDog)) {
						player = new Dog(playerId);
						human = false;
					} else if (playerConfig.name.equals(Names.kMouse)) {
						player = new Mouse(playerId);
						human = false;
					} else {
						player = new Robot(playerId);
					}
					break;

				case KITCHEN:
					if (Soar2D.config.generalConfig().tosca) {
						player = new ToscaCook(playerId);
						human = false;
					} else {
						player = new Cook(playerId);
					}
					break;
					
				case TAXI:
					player = new Taxi(playerId);
					break;

				}
				
				assert player != null;
				
				// set its location if necessary
				int [] initialLocation = playerConfig.pos == null ? null : new int [] { playerConfig.pos[0], playerConfig.pos[1] };

				// This can fail if there are no open squares on the map, message printed already
				if (!world.addPlayer(player, initialLocation, human)) {
					throw new CreationException();
				}
				
			} else {
				
				// we need to create a soar agent, do it
				Agent agent = kernel.CreateAgent(playerConfig.name);
				if (agent == null) {
					throw new CreationException("Agent " + playerConfig.name + " creation failed: " + kernel.GetLastErrorDescription());
				}
				
				try {
					// now load the productions
					File productionsFile = new File(playerConfig.productions);
					if (!agent.LoadProductions(productionsFile.getAbsolutePath())) {
						throw new CreationException("Agent " + playerConfig.name + " production load failed: " + agent.GetLastErrorDescription());
					}
					
					// if requested, silence agent
					if (Soar2D.config.soarConfig().watch_0) {
						agent.ExecuteCommandLine("watch 0");
					}
					
					// if requested, set max memory usage
					int maxmem = Soar2D.config.soarConfig().max_memory_usage;
					if (maxmem > 0) {
						agent.ExecuteCommandLine("max-memory-usage " + Integer.toString(maxmem));
					}
			
					Player player = null;
					
					// create the tank or eater, soar style
					switch(Soar2D.config.game()) {
					case EATERS:
						player = new SoarEater(agent, playerId); 
						break;
					case TANKSOAR:
						player = new SoarTank(agent, playerId);
						break;
					case ROOM:
						player = new SoarRobot(agent, playerId);
						break;
					case KITCHEN:
						player = new SoarCook(agent, playerId);
						break;
					case TAXI:
						player = new SoarTaxi(agent, playerId);

					}
					
					assert player != null;
					
					// handle the initial location if necessary
					int [] initialLocation = playerConfig.pos == null ? null : new int [] { playerConfig.pos[0], playerConfig.pos[1] };
					
					// This can fail if there are no open squares on the map, message printed already
					if (!world.addPlayer(player, initialLocation, false)) {
						throw new CreationException();
					}
		
					// Scott Lathrop --  register for print events
					if (Soar2D.config.soarConfig().soar_print) {
						agent.RegisterForPrintEvent(smlPrintEventId.smlEVENT_PRINT, Soar2D.control.getLogger(), null,true);
					}
					
					// save the agent
					agents.put(player.getName(), agent);
					
					// spawn the debugger if we're supposed to
					if (Soar2D.config.soarConfig().spawn_debuggers && !isClientConnected(Names.kDebuggerClient)) {
						ClientConfig debuggerConfig = Soar2D.config.clientConfigs().get(Names.kDebuggerClient);
						debuggerConfig.command = getDebuggerCommand(player.getName());

						spawnClient(Names.kDebuggerClient, debuggerConfig);
					}
					
				} catch (CreationException e) {
					// A problem in this block requires agent deletion
					kernel.DestroyAgent(agent);
					agent.delete();
					throw e;
				}
			}
		} catch (CreationException e) {
			// a problem in this block requires us to free up the color
			freeAColor(playerConfig.color);
			if (e.getMessage() != null) {
				error(e.getMessage());
			}
			return;
		}
		
		// the agent list has changed, notify things that care
		Soar2D.control.playerEvent();
	}
	
	/**
	 * @param client the client in question
	 * @return true if it is connected
	 * 
	 * check to see if the client specified by the client config is connected or not
	 */
	public boolean isClientConnected(String clientId) {
		boolean connected = false;
		kernel.GetAllConnectionInfo();
		for (int i = 0; i < kernel.GetNumberConnections(); ++i) {
			ConnectionInfo info =  kernel.GetConnectionInfo(i);
			if (info.GetName().equalsIgnoreCase(clientId)) {
				connected = true;
				break;
			}
		}
		return connected;
	}
	
	/**
	 * @param agentName tailor the command to this agent name
	 * @return a string command line to execute to spawn the debugger
	 */
	public String getDebuggerCommand(String agentName) {
		// Figure out whether to use java or javaw
		String os = System.getProperty("os.name");
		String commandLine;
		if (os.matches(".+indows.*") || os.matches("INDOWS")) {
			commandLine = "javaw -jar \"" + getBasePath() 
			+ "..\\..\\SoarLibrary\\bin\\SoarJavaDebugger.jar\" -cascade -remote -agent " 
			+ agentName + " -port " + Soar2D.config.soarConfig().port;
		} else {
			commandLine = System.getProperty("java.home") + "/bin/java -jar " + getBasePath()
			+ "../../SoarLibrary/bin/SoarJavaDebugger.jar -XstartOnFirstThread -cascade -remote -agent " 
			+ agentName + " -port " + Soar2D.config.soarConfig().port;
		}
		
		return commandLine;
	}

	/**
	 * @param player the player to remove
	 * 
	 * removes the player from the world and blows away any associated data, 
	 * frees up its color, etc.
	 */
	public void destroyPlayer(Player player) {
		// remove it from the world, can't fail
		world.removePlayer(player.getName());
		
		// free its color
		freeAColor(player.getColor());
		
		// call its shutdown
		player.shutdown();
		
		// get the agent (human agents return null here)
		Agent agent = agents.remove(player.getName());
		if (agent != null) {
			// there was an agent, destroy it
			kernel.DestroyAgent(agent);
			agent.delete();
			agent = null;
		}
		
		// the player list has changed, notify those who care
		Soar2D.control.playerEvent();
	}
	
	/**
	 * @param player the player to reload
	 * 
	 * reload the player. only currently makes sense to reload a soar agent.
	 * this re-loads the productions
	 */
	public void reloadPlayer(Player player) {
		Agent agent = agents.get(player.getName());
		if (agent == null) {
			return;
		}
		
		PlayerConfig playerConfig = Soar2D.config.playerConfigs().get(player.getID());
		assert playerConfig != null;
		assert playerConfig.productions != null;
		File productionsFile = new File(playerConfig.productions);
		agent.LoadProductions(productionsFile.getAbsolutePath());
	}
	
	/**
	 * @param after do the clients denoted as "after" agent creation
	 * @return true if the clients all connected.
	 */
	private boolean doClients(boolean after) {
		for ( Entry<String, ClientConfig> entry : Soar2D.config.clientConfigs().entrySet()) {
			if (entry.getValue().after != after) {
				continue;
			}
			
			if (entry.getKey().equals(Names.kDebuggerClient)) {
				continue;
			}
			
			if (entry.getValue().command != null) {
				spawnClient(entry.getKey(), entry.getValue());
			} else {
				if (!waitForClient(entry.getKey(), entry.getValue())) {
					error(Names.Errors.clientSpawn + entry.getKey());
					return false;
				}
			}
		}
		return true;
	}

	/**
	 * @author voigtjr
	 *
	 * This handles some nitty gritty client spawn stuff
	 */
	private class Redirector extends Thread {
		BufferedReader br;
		public Redirector(BufferedReader br) {
			this.br = br;
		}
		
		public void run() {
			String line;
			try {
				while ((line = br.readLine()) != null) {
					System.out.println(line);
				}
			} catch (IOException e) {
				System.err.println(e.getMessage());
			}
		}
	}
	
	/**
	 * @param client the client to spawn
	 * 
	 * spawns a client, waits for it to connect
	 */
	public void spawnClient(String clientID, ClientConfig clientConfig) {
		Runtime r = java.lang.Runtime.getRuntime();
		logger.trace(Names.Trace.spawningClient + clientID);

		try {
			Process p = r.exec(clientConfig.command);
			
			InputStream is = p.getInputStream();
			InputStreamReader isr = new InputStreamReader(is);
			BufferedReader br = new BufferedReader(isr);
			Redirector rd = new Redirector(br);
			rd.start();

			is = p.getErrorStream();
			isr = new InputStreamReader(is);
			br = new BufferedReader(isr);
			rd = new Redirector(br);
			rd.start();
			
			if (!waitForClient(clientID, clientConfig)) {
				error(Names.Errors.clientSpawn + clientID);
				return;
			}
			
		} catch (IOException e) {
			error(Names.Errors.clientSpawn + clientID + ": " + e.getMessage());
			shutdown();
			System.exit(1);
		}
	}
	
	/**
	 * @param client the client to wait for
	 * @return true if the client connected within the timeout
	 * 
	 * waits for a client to report ready
	 */
	public boolean waitForClient(String clientID, ClientConfig clientConfig) {
		boolean ready = false;
		// do this loop if timeout seconds is 0 (code for wait indefinitely) or if we have tries left
		for (int tries = 0; (clientConfig.timeout == 0) || (tries < clientConfig.timeout); ++tries) {
			kernel.GetAllConnectionInfo();
			if (kernel.HasConnectionInfoChanged()) {
				for (int i = 0; i < kernel.GetNumberConnections(); ++i) {
					ConnectionInfo info =  kernel.GetConnectionInfo(i);
					if (info.GetName().equalsIgnoreCase(clientID)) {
						if (info.GetAgentStatus().equalsIgnoreCase(sml_Names.getKStatusReady())) {
							ready = true;
							break;
						}
					}
				}
				if (ready) {
					break;
				}
			}
			try { 
				logger.trace(Names.Trace.waitClient + clientID);
				Thread.sleep(1000); 
			} catch (InterruptedException ignored) {}
		}
		return ready;
	}
	
	/**
	 * update the sim, or, in this case, the world
	 */
	public void update() {
		world.update();
	}

	/**
	 * run soar forever
	 */
	public void runForever() {
		if (runTilOutput) {
			kernel.RunAllAgentsForever(smlRunStepSize.sml_UNTIL_OUTPUT);
		} else {
			kernel.RunAllAgentsForever();
		}
		
	}

	/**
	 * run soar one step
	 */
	public void runStep() {
		if (runTilOutput) {
			kernel.RunAllTilOutput(smlRunStepSize.sml_UNTIL_OUTPUT);
		} else {
			kernel.RunAllAgents(1);
		}
	}

	/**
	 * @return true if the map reset was successful
	 * 
	 * resets the world, ready for a new run
	 */
	public boolean reset() {
		logger.info(Names.Info.reset);
		if (!world.load()) {
			File mapFile = new File(Soar2D.config.generalConfig().map);
			error("Error loading map " + mapFile.getAbsolutePath());
			return false;
		}
		return true;
	}

	/**
	 * shuts things down, including the kernel, in preparation for an exit to dos
	 */
	public void shutdown() {
		if (world != null) {
			world.shutdown();
		}
		
		assert this.agents.size() == 0;
		
		if (kernel != null) {
			logger.trace(Names.Trace.kernelShutdown);
			kernel.Shutdown();
			kernel.delete();
		}
	}
	
	/**
	 * @return true if there are human agents present
	 */
	public boolean hasHumanAgents() {
		return agents.size() < world.getPlayers().size();
	}
	
	/**
	 * @return true if there are soar agents present
	 */
	public boolean hasSoarAgents() {
		return agents.size() > 0;
	}
	
	/**
	 * TODO
	 * @return true if the simulation has reached a terminal state
	 * 
	 * check to see if one of the terminal states has been reached
	 */
	public boolean isDone() {
		return world.isTerminal();
	}

	public String getBasePath() {
		return System.getProperty("user.dir") + System.getProperty("file.separator");
	}
	public String getMapPath() {
		return Soar2D.simulation.getBasePath() + "maps" + System.getProperty("file.separator");
	}
	public String getMapExt() {
		switch (Soar2D.config.game()) {
		case TANKSOAR:
			return "tmap";
		case EATERS:
			return "emap";
		case ROOM:
			return "bmap";
		case KITCHEN:
		case TAXI:
			return "xml";
		}
		return null;
	}
	public String getAgentPath() {
		return Soar2D.simulation.getBasePath() + "agents" + System.getProperty("file.separator");
	}
}
