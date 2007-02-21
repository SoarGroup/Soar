package soar2d;

import java.io.*;
import java.util.*;
import java.util.logging.*;

import sml.*;
import soar2d.Configuration.SimType;
import soar2d.player.*;

/**
 * @author voigtjr
 *
 * Keeps track of the meta simulation state. The world keeps track of more state and
 * is the major member of this class. Creates the soar kernel and registers events.
 */
public class Simulation {

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
	public final String kColors[] = { "red", "blue", "purple", "yellow", "orange", "green", "black",  };
	/**
	 * A list of colors not currently taken by a player
	 */
	private ArrayList<String> unusedColors = new ArrayList<String>(kColors.length);
	/**
	 * String agent name to agent mapping
	 */
	private HashMap<String, Agent> agents = new HashMap<String, Agent>();
	/**
	 * String agent name to player config mapping
	 */
	private HashMap<String, PlayerConfig> configs = new HashMap<String, PlayerConfig>();

	private static final String kDog = "dog";
	private static final String kMouse = "mouse";
	
	/**
	 * @return true if there were no errors during initialization
	 * 
	 * sets everything up in preparation of execution. only called once per
	 * program run (not once per soar run)
	 */
	public boolean initialize() {
		// keep track of colors
		for (int i = 0; i < kColors.length; ++i) {
			unusedColors.add(kColors[i]);
		}
		
		// Tanksoar uses run til output
		runTilOutput = (Soar2D.config.getType() == SimType.kTankSoar);
		
		// Initialize Soar
		if (Soar2D.config.getRemote()) {
			kernel = Kernel.CreateRemoteConnection(true);
		} else {
			// Create kernel
			kernel = Kernel.CreateKernelInNewThread("SoarKernelSML", 12121);
			//kernel = Kernel.CreateKernelInCurrentThread("SoarKernelSML", true);
		}

		if (kernel.HadError()) {
			Soar2D.control.severeError("Error creating kernel: " + kernel.GetLastErrorDescription());
			return false;
		}
		
		// We want the most performance
		if (Soar2D.logger.isLoggable(Level.FINEST)) Soar2D.logger.finest("Setting auto commit false.");
		kernel.SetAutoCommit(false);

		// Make all runs non-random if asked
		// For debugging, set this to make all random calls follow the same sequence
		if (Soar2D.config.hasRandomSeed()) {
			if (Soar2D.logger.isLoggable(Level.FINEST)) Soar2D.logger.finest("Not seeding generators.");
			random = new Random();
		} else {
			// seed the generators
			if (Soar2D.logger.isLoggable(Level.FINEST)) Soar2D.logger.finest("Seeding generators with " + Soar2D.config.getRandomSeed());
			kernel.ExecuteCommandLine("srand " + Soar2D.config.getRandomSeed(), null) ;
			random = new Random(Soar2D.config.getRandomSeed());
		}
		
		// Register for events
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, Soar2D.control, null);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, Soar2D.control, null);
		if (runTilOutput) {
			if (Soar2D.logger.isLoggable(Level.FINEST)) Soar2D.logger.finest("Registering for: smlEVENT_AFTER_ALL_GENERATED_OUTPUT");
			kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_GENERATED_OUTPUT, Soar2D.control, null);
		} else {
			if (Soar2D.logger.isLoggable(Level.FINEST)) Soar2D.logger.finest("Registering for: smlEVENT_AFTER_ALL_OUTPUT_PHASES");
			kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, Soar2D.control, null);
		}
		
		// Load the world
		if(!world.load()) {
			return false;
		}
		
		// Start or wait for clients (false: before agent creation)
		if (!doClients(false)) {
			return false;
		}
		
		// add dog and mouse (book)
		// TODO: this could happen in the config file
		if (Soar2D.config.getType() == SimType.kBook) {
			PlayerConfig dogConfig = new PlayerConfig();
			dogConfig.setName(kDog);
			createPlayer(dogConfig);
			
			PlayerConfig mouseConfig = new PlayerConfig();
			mouseConfig.setName(kMouse);
			createPlayer(mouseConfig);
		}
		
		// add initial players
		Iterator<PlayerConfig> iter = Soar2D.config.getPlayers().iterator();
		while (iter.hasNext()) {
			createPlayer(iter.next());
		}

		// Start or wait for clients (true: after agent creation)
		if (!doClients(true)) {
			return false;
		}
		
		// success
		return true;
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
		for (int i = 0; i < kColors.length; ++i) {
			if (color.equals(kColors[i])) {
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
	public void createPlayer(PlayerConfig playerConfigIn) {
		
		PlayerConfig playerConfig = new PlayerConfig(playerConfigIn);
		
		// if a color was specified
		if (playerConfig.hasColor()) {
			//make sure it is unused
			if (!unusedColors.contains(playerConfig.getColor())) {
				Soar2D.control.severeError("Color used or not available: " + playerConfig.getColor());
				return;
			}
			// it is unused, so use it
			useAColor(playerConfig.getColor());
		} else {
			
			// no color specified, pick on at random
			String color = useAColor(null);
			
			// make sure we got one
			if (color == null) {
				
				// if we didn't then they are all gone
				Soar2D.control.severeError("There are no more player slots available.");
				return;
			}
			
			// set it
			playerConfig.setColor(color);
		}

		// if we don't have a name
		if (!playerConfig.hasName()) {
			// then use our color
			playerConfig.setName(playerConfig.getColor());
		}
		
		try {
			// check for duplicate name
			if (configs.containsKey(playerConfig.getName())) {
				throw new CreationException("Failed to create player: " + playerConfig.getName() + " already exists.");
			}
			
			// check for human agent
			if (playerConfig.hasProductions() == false) {
				
				// create a human agent
				Player player = null;
				
				// eater or tank depending on the setting
				switch(Soar2D.config.getType()) {
				case kEaters:
					player = new Eater(playerConfig, true);
					break;
				case kTankSoar:
					player = new Tank(playerConfig);
					break;
				case kBook:
					if (playerConfig.getName().equals(kDog)) {
						player = new Dog(playerConfig);
					} else if (playerConfig.getName().equals(kMouse)) {
						player = new Mouse(playerConfig);
					} else {
						player = new Cat(playerConfig);
					}
					break;
				}
				
				assert player != null;
				
				// set its location if necessary
				java.awt.Point initialLocation = null;
				if (playerConfig.hasInitialLocation()) {
					initialLocation = playerConfig.getInitialLocation();
				}

				// This can fail if there are no open squares on the map, message printed already
				if (!world.addPlayer(player, initialLocation, true)) {
					throw new CreationException();
				}

				// save the player configuration
				configs.put(player.getName(), playerConfig);
				
			} else {
				
				// we need to create a soar agent, do it
				Agent agent = kernel.CreateAgent(playerConfig.getName());
				if (agent == null) {
					throw new CreationException("Agent " + playerConfig.getName() + " creation failed: " + kernel.GetLastErrorDescription());
				}
				
				try {
					// now load the productions
					if (!agent.LoadProductions(playerConfig.getProductions().getAbsolutePath())) {
						throw new CreationException("Agent " + playerConfig.getName() + " production load failed: " + agent.GetLastErrorDescription());
					}
			
					Player player = null;
					
					// create the tank or eater, soar style
					switch(Soar2D.config.getType()) {
					case kEaters:
						player = new SoarEater(agent, playerConfig); 
						break;
					case kTankSoar:
						player = new SoarTank(agent, playerConfig);
						break;
					case kBook:
						assert false;
						break;
					}
					
					assert player != null;
					
					// handle the initial location if necesary
					java.awt.Point initialLocation = null;
					if (playerConfig.hasInitialLocation()) {
						initialLocation = playerConfig.getInitialLocation();
					}
					
					// This can fail if there are no open squares on the map, message printed already
					if (!world.addPlayer(player, initialLocation, false)) {
						throw new CreationException();
					}
		
					// save both the agent
					agents.put(player.getName(), agent);
					
					// and the configuration
					configs.put(player.getName(), playerConfig);
					
					// spawn the debugger if we're supposed to
					Names.kDebuggerClient.command = getDebuggerCommand(player.getName());
					if (Soar2D.config.getDebuggers() && !isClientConnected(Names.kDebuggerClient)) {
						spawnClient(Names.kDebuggerClient);
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
			freeAColor(playerConfig.getColor());
			if (e.getMessage() != null) {
				Soar2D.control.severeError(e.getMessage());
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
	public boolean isClientConnected(ClientConfig client) {
		boolean connected = false;
		kernel.GetAllConnectionInfo();
		for (int i = 0; i < kernel.GetNumberConnections(); ++i) {
			ConnectionInfo info =  kernel.GetConnectionInfo(i);
			if (info.GetName().equalsIgnoreCase(client.name)) {
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
			commandLine = "javaw -jar \"" + Soar2D.config.getBasePath() 
			+ "..\\..\\SoarLibrary\\bin\\SoarJavaDebugger.jar\" -cascade -remote -agent " + agentName;
		} else {
			commandLine = System.getProperty("java.home") + "/bin/java -jar " + Soar2D.config.getBasePath() 
			+ "../../SoarLibrary/bin/SoarJavaDebugger.jar -cascade -remote -agent " + agentName;
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
		
		// remove it from the config list
		configs.remove(player.getName());
		
		// free its color
		freeAColor(player.getColor());
		
		// call its shutdown
		player.shutdown();
		
		// get the agent (human agents return null here)
		Agent agent = agents.remove(player.getName());
		if (agent != null) {
			// there was an agent, destroy it
			if (!kernel.DestroyAgent(agent)) {
				Soar2D.control.severeError("Failed to destroy soar agent " + player.getName() + ": " + kernel.GetLastErrorDescription());
			}
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
		PlayerConfig config = configs.get(player.getName());
		assert config != null;
		assert config.hasProductions();
		agent.LoadProductions(config.getProductions().getAbsolutePath());
	}
	
	/**
	 * @param name the name of the player to clone
	 * @param newName the name of the new player
	 * 
	 * This creates a player similar to a current one. the name changes, the color
	 * changes. This doesn't necessarily succeed. control playerEvent is 
	 * called on success.
	 */
	public void clonePlayer(String name, String newName) {
		PlayerConfig config = configs.get(name);
		assert config != null;
		config.setName(newName);
		config.setColor(null);
		createPlayer(config);
	}

	/**
	 * @param after do the clients denoted as "after" agent creation
	 * @return true if the clients all connected.
	 */
	private boolean doClients(boolean after) {
		Iterator<ClientConfig> clientIter = Soar2D.config.clients.iterator();
		while (clientIter.hasNext()) {
			ClientConfig client = clientIter.next();
			if (client.after == after) {
				continue;
			}
			if (client.command != null) {
				spawnClient(client);
			} else {
				if (!waitForClient(client)) {
					Soar2D.control.severeError("Client spawn failed: " + client.name);
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
	public void spawnClient(ClientConfig client) {
		Runtime r = java.lang.Runtime.getRuntime();
		if (Soar2D.logger.isLoggable(Level.FINER)) Soar2D.logger.finer("Spawning client: " + client.command);

		try {
			Process p = r.exec(client.command);
			
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
			
			if (!waitForClient(client)) {
				Soar2D.control.severeError("Client spawn failed: " + client.name);
				return;
			}
			
		} catch (IOException e) {
			Soar2D.control.severeError("IOException spawning client: " + client.name + ": " + e.getMessage());
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
	public boolean waitForClient(ClientConfig client) {
		boolean ready = false;
		// do this loop if timeout seconds is 0 (code for wait indefinitely) or if we have tries left
		for (int tries = 0; (client.timeout == 0) || (tries < client.timeout); ++tries) {
			kernel.GetAllConnectionInfo();
			if (kernel.HasConnectionInfoChanged()) {
				for (int i = 0; i < kernel.GetNumberConnections(); ++i) {
					ConnectionInfo info =  kernel.GetConnectionInfo(i);
					if (info.GetName().equalsIgnoreCase(client.name)) {
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
				if (Soar2D.logger.isLoggable(Level.FINEST)) Soar2D.logger.finest("Waiting for client: "+ client.name);
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
			kernel.RunAllAgentsForever(smlInterleaveStepSize.sml_INTERLEAVE_UNTIL_OUTPUT);
		} else {
			kernel.RunAllAgentsForever();
		}
		
	}

	/**
	 * run soar one step
	 */
	public void runStep() {
		if (runTilOutput) {
			kernel.RunAllTilOutput();
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
		Soar2D.logger.info("Resetting simulation.");
		if (!world.load()) {
			Soar2D.control.severeError("Error loading map " + Soar2D.config.getMap().getAbsolutePath());
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
		assert this.configs.size() == 0;
		
		if (kernel != null) {
			if (Soar2D.logger.isLoggable(Level.FINEST)) Soar2D.logger.finest("Shutting down kernel.");
			kernel.Shutdown();
			kernel.delete();
		}
	}
	
	/**
	 * @return true if there are human agents present
	 */
	public boolean hasHumanAgents() {
		return agents.size() < this.configs.size();
	}
	
	/**
	 * @return true if there are soar agents present
	 */
	public boolean hasSoarAgents() {
		return agents.size() > 0;
	}
	
	/**
	 * @return true if there are any players present (not necessarily soar agents)
	 */
	public boolean hasPlayers() {
		return world.hasPlayers();
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
}
