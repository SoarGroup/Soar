package soar2d;

import java.io.*;
import java.util.*;
import java.util.logging.*;

import sml.*;
import soar2d.player.Player;
import soar2d.player.PlayerConfig;
import soar2d.player.SoarEater;

public class Simulation {

	boolean runTilOutput = false;
	Kernel kernel = null;
	public static Random random = null;
	public World world = new World();
	String currentMap = null;
	private final String kColors[] = { "red", "blue", "purple", "yellow", "orange", "black", "green" };
	private ArrayList<String> unusedColors = new ArrayList<String>(kColors.length);
	HashMap<String, Agent> agents = new HashMap<String, Agent>();
	HashMap<String, PlayerConfig> configs = new HashMap<String, PlayerConfig>();
	
	public boolean initialize() {
		for (int i = 0; i < kColors.length; ++i) {
			unusedColors.add(kColors[i]);
		}
		
		// Tanksoar uses run til output
		runTilOutput = Soar2D.config.tanksoar;
		
		// Initialize Soar
		if (Soar2D.config.remote) {
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
		if (Soar2D.config.random) {
			if (Soar2D.logger.isLoggable(Level.FINEST)) Soar2D.logger.finest("Not seeding generator.");
			random = new Random();
		} else {
			if (Soar2D.logger.isLoggable(Level.FINER)) Soar2D.logger.finer("Seeding generator 0.");
			kernel.ExecuteCommandLine("srand 0", null) ;
			random = new Random(0);
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
		currentMap = Soar2D.config.mapPath + Soar2D.config.map;
		world.load(currentMap);
		
		// Start or wait for clients (false: before agent creation)
		if (!doClients(false)) {
			return false;
		}
		
		// add initial players
		Iterator<PlayerConfig> iter = Soar2D.config.players.iterator();
		while (iter.hasNext()) {
			createPlayer(iter.next());
		}

		// Start or wait for clients (true: after agent creation)
		if (!doClients(true)) {
			return false;
		}
		
		return true;
	}
	
	public ArrayList<String> getUnusedColors() {
		return unusedColors;
	}
	
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
	
	public boolean freeAColor(String color) {
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

	public void createPlayer(PlayerConfig playerConfig) {
		if (playerConfig.hasColor()) {
			if (!unusedColors.contains(playerConfig.getColor())) {
				Soar2D.control.severeError("Color used or not available: " + playerConfig.getColor());
				return;
			}
			useAColor(playerConfig.getColor());
		} else {
			String color = useAColor(null);
			if (color == null) {
				Soar2D.control.severeError("There are no more player slots available.");
				return;
			}
			playerConfig.setColor(color);
		}
		
		if (!playerConfig.hasName()) {
			playerConfig.setName(playerConfig.getColor());
		}
		
		if (configs.containsKey(playerConfig.getName())) {
			freeAColor(playerConfig.getColor());
			Soar2D.control.severeError("Failed to create player: " + playerConfig.getName() + " already exists.");
			return;
		}
		
		if (playerConfig.hasProductions() == false) {
			// human player
			freeAColor(playerConfig.getColor());
			Soar2D.control.severeError("Human player not yet supported.");
			return;
		}
		Agent agent = kernel.CreateAgent(playerConfig.getName());
		if (agent == null) {
			freeAColor(playerConfig.getColor());
			Soar2D.control.severeError("Agent " + playerConfig.getName() + " creation failed: " + kernel.GetLastErrorDescription());
			return;
		}
		
		
		if (!agent.LoadProductions(playerConfig.getProductions().getAbsolutePath())) {
			freeAColor(playerConfig.getColor());
			Soar2D.control.severeError("Agent " + playerConfig.getName() + " production load failed: " + agent.GetLastErrorDescription());
			return;
		}

		if (Soar2D.config.eaters) {
			SoarEater eater = new SoarEater(agent, playerConfig); 
			agents.put(eater.getName(), agent);
			configs.put(eater.getName(), playerConfig);
			java.awt.Point initialLocation = null;
			if (playerConfig.hasInitialLocation()) {
				initialLocation = playerConfig.getInitialLocation();
			}
			world.addPlayer(eater, initialLocation);
			
			Names.kDebuggerClient.command = getDebuggerCommand(eater.getName());
			if (Soar2D.config.debuggers && !isClientConnected(Names.kDebuggerClient)) {
				spawnClient(Names.kDebuggerClient);
			}

		} else {
			freeAColor(playerConfig.getColor());
			Soar2D.control.severeError("TankSoar player not yet supported.");
			return;
		}
		Soar2D.control.playerEvent();
	}
	
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
	
	public String getDebuggerCommand(String agentName) {
		// Figure out whether to use java or javaw
		String os = System.getProperty("os.name");
		String commandLine;
		if (os.matches(".+indows.*") || os.matches("INDOWS")) {
			commandLine = "javaw -jar \"" + Soar2D.config.basePath 
			+ "..\\..\\SoarLibrary\\bin\\SoarJavaDebugger.jar\" -remote -agent " + agentName;
		} else {
			commandLine = System.getProperty("java.home") + "/bin/java -jar " + Soar2D.config.basePath
			+ "../../SoarLibrary/bin/SoarJavaDebugger.jar -remote -agent " + agentName;
		}
		
		return commandLine;
	}

	public void destroyPlayer(Player player) {
		world.removePlayer(player.getName());
		configs.remove(player.getName());
		Agent agent = agents.remove(player.getName());
		if (agent != null) {
			if (!kernel.DestroyAgent(agent)) {
				Soar2D.control.severeError("Failed to destroy soar agent " + player.getName() + ": " + kernel.GetLastErrorDescription());
			}
		}
		agent.delete();
		agent = null;
		Soar2D.control.playerEvent();
	}
	
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
	
	public void clonePlayer(String name, String newName) {
		PlayerConfig config = configs.get(name);
		assert config != null;
		config.setName(newName);
		config.setColor(null);
		createPlayer(config);
	}

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
	
	public void update() {
		world.update();
	}

	public void runForever() {
		if (runTilOutput) {
			kernel.RunAllAgentsForever(smlInterleaveStepSize.sml_INTERLEAVE_UNTIL_OUTPUT);
		} else {
			kernel.RunAllAgentsForever();
		}
		
	}

	public void runStep() {
		if (runTilOutput) {
			kernel.RunAllTilOutput();
		} else {
			kernel.RunAllAgents(1);
		}
	}

	public void reset() {
		if (Soar2D.logger.isLoggable(Level.INFO)) Soar2D.logger.info("Resetting simulation.");
		if (!world.load(currentMap)) {
			Soar2D.control.severeError("Error loading map " + currentMap);
		}
		world.reset();
	}

	public void shutdown() {
		if (world != null) {
			world.shutdown();
		}
		if (kernel != null) {
			if (Soar2D.logger.isLoggable(Level.FINER)) Soar2D.logger.finer("Shutting down kernel.");
			kernel.Shutdown();
			kernel.delete();
		}
	}
	
	public boolean hasSoarAgents() {
		return agents.size() > 0;
	}
	
	public boolean hasPlayers() {
		return world.hasPlayers();
	}

	public boolean isDone() {
		// TODO This should test the goals. Controls whether Run and Step are available.
		return false;
	}
}
