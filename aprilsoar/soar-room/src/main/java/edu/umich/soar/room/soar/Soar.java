package edu.umich.soar.room.soar;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.prefs.Preferences;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.commsen.stopwatch.Report;
import com.commsen.stopwatch.Stopwatch;

import edu.umich.soar.room.Application;
import edu.umich.soar.room.config.ClientConfig;
import edu.umich.soar.room.config.SimConfig;
import edu.umich.soar.room.config.SoarConfig;
import edu.umich.soar.room.core.CognitiveArchitecture;
import edu.umich.soar.room.core.Names;
import edu.umich.soar.room.core.Simulation;
import edu.umich.soar.room.core.events.AfterTickEvent;
import edu.umich.soar.room.core.events.BeforeTickEvent;
import edu.umich.soar.room.core.events.ResetEvent;
import edu.umich.soar.room.core.events.StartEvent;
import edu.umich.soar.room.core.events.StopEvent;
import edu.umich.soar.room.events.SimEvent;
import edu.umich.soar.room.events.SimEventListener;
import edu.umich.soar.room.map.Robot;
import edu.umich.soar.room.map.RobotCommander;
import edu.umich.soar.room.map.RoomWorld;

import sml.Agent;
import sml.ConnectionInfo;
import sml.Kernel;
import sml.smlUpdateEventId;
import sml.sml_Names;

public class Soar implements CognitiveArchitecture, Kernel.UpdateEventInterface, SimEventListener {

	private static Log logger = LogFactory.getLog(Soar.class);

	private Kernel kernel = null;

	private class AgentData {
		AgentData(Agent agent, File productions) {
			this.agent = agent;
			this.productions = productions;
		}

		Agent agent;
		File productions;
		SoarAgent sa;
	}

	private final Map<String, AgentData> agents = new HashMap<String, AgentData>();
	private final Map<String, ClientConfig> clients;
	private final int maxMemoryUsage;
	private final int port;
	private boolean debug;
	private final Simulation sim;
	private final Preferences pref;
	private final String KEY_ASYNC = "async";

	/**
	 * @param config
	 * @param clients
	 * @param game
	 * @param basePath
	 * 
	 * @throws IllegalStateException
	 *             If there is an unrecoverable error initializing Soar
	 */
	public Soar(Simulation sim) {
		this.sim = sim;
		SoarConfig config = sim.getConfig().soarConfig();

		this.clients = sim.getConfig().clientConfigs();
		this.maxMemoryUsage = config.max_memory_usage;
		this.port = config.port;
		this.debug = config.spawn_debuggers;
		this.pref = Application.PREFERENCES.node("soar");
		this.async = pref.getBoolean(KEY_ASYNC, true);

		if (config.remote != null) {
			kernel = Kernel.CreateRemoteConnection(true, config.remote,
					config.port);
		} else {
			// Create kernel
			kernel = Kernel.CreateKernelInNewThread("SoarKernelSML", config.port);
			//kernel = Kernel.CreateKernelInCurrentThread("SoarKernelSML", true);
		}

		if (kernel.HadError()) {
			throw new IllegalStateException(Names.Errors.kernelCreation
					+ kernel.GetLastErrorDescription());
		}

		// We want the most performance
		logger.debug(Names.Debug.autoCommit);
		kernel.SetAutoCommit(false);

		// Register for Soar events
		logger.trace(Names.Trace.eventRegistration);
		logger.debug(Names.Debug.noRunTilOutput);
		kernel.RegisterForUpdateEvent(
				smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, this,
				null);

		// Register for Sim events
		sim.getEvents().addListener(StartEvent.class, this);
		sim.getEvents().addListener(StopEvent.class, this);
		sim.getEvents().addListener(BeforeTickEvent.class, this);
		sim.getEvents().addListener(AfterTickEvent.class, this);
		sim.getEvents().addListener(ResetEvent.class, this);
	}

	@Override
	public boolean debug() {
		return debug;
	}

	@Override
	public void seed(int seed) {
		kernel.ExecuteCommandLine("srand " + seed, null);
	}

	private class Redirector extends Thread {
		BufferedReader br;

		public Redirector(BufferedReader br) {
			this.br = br;
		}

		@Override
		public void run() {
			String line;
			try {
				while ((line = br.readLine()) != null) {
					System.out.println(line);
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

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
				sim.error("Soar", Names.Errors.clientSpawn + clientID);
				return;
			}

		} catch (IOException e) {
			e.printStackTrace();
			sim.error("Soar", "IOException spawning client: " + clientID);
			return;
		}
	}

	/**
	 * @param client
	 *            the client to wait for
	 * @return true if the client connected within the timeout
	 * 
	 *         waits for a client to report ready
	 */
	public boolean waitForClient(String clientID, ClientConfig clientConfig) {
		boolean ready = false;
		// do this loop if timeout seconds is 0 (code for wait indefinitely) or
		// if we have tries left
		for (int tries = 0; (clientConfig.timeout == 0)
				|| (tries < clientConfig.timeout); ++tries) {
			kernel.GetAllConnectionInfo();
			if (kernel.HasConnectionInfoChanged()) {
				for (int i = 0; i < kernel.GetNumberConnections(); ++i) {
					ConnectionInfo info = kernel.GetConnectionInfo(i);
					if (info.GetName().equalsIgnoreCase(clientID)) {
						if (info.GetAgentStatus().equalsIgnoreCase(
								sml_Names.getKStatusReady())) {
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
			} catch (InterruptedException ignored) {
			}
		}
		return ready;
	}

	@Override
	public void destroyPlayer(String name) {
		// get the agent (human agents return null here)
		AgentData agentData = agents.remove(name);
		if (agentData == null) {
			return;
		}
		// there was an agent, destroy it
		kernel.DestroyAgent(agentData.agent);
		agentData.agent.delete();
	}

	@Override
	public void shutdown() {
		pref.putBoolean(KEY_ASYNC, async);
		
		for (Report report : Stopwatch.getAllReports()) {
			System.out.println(report);
		}
		
		exec.shutdown();
		try {
			exec.awaitTermination(5, TimeUnit.MINUTES);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		
		for (AgentData ad : agents.values()) {
			kernel.DestroyAgent(ad.agent);
		}
		
		if (kernel != null) {
			logger.trace(Names.Trace.kernelShutdown);
			kernel.Shutdown();
		}
	}

	private Agent createSoarAgent(String name, String productions, boolean spawnDebuggers) {
		Agent agent = kernel.CreateAgent(name);
		if (agent == null) {
			sim.error("Soar", "Error creating agent " + name + ", "
					+ kernel.GetLastErrorDescription());
			return null;
		}

		// now load the productions
		File productionsFile = new File(productions);
		if (!agent.LoadProductions(productionsFile.getAbsolutePath())) {
			sim.error("Soar", "Error loading productions " + productionsFile
					+ " for " + name + ", " + agent.GetLastErrorDescription());
			kernel.DestroyAgent(agent);
			return null;
		}

		// if requested, set max memory usage
		int maxmem = maxMemoryUsage;
		if (maxmem > 0) {
			agent.ExecuteCommandLine("max-memory-usage "
					+ Integer.toString(maxmem));
		}

		// save the agent
		agents.put(name, new AgentData(agent, productionsFile));

		// spawn the debugger if we're supposed to
		if (spawnDebuggers && !isClientConnected(Names.kDebuggerClient)) {
			ClientConfig debuggerConfig = clients.get(Names.kDebuggerClient);
			debuggerConfig.command = getDebuggerCommand(name);

			spawnClient(Names.kDebuggerClient, debuggerConfig);
		}

		return agent;
	}

	/**
	 * @param client
	 *            the client in question
	 * @return true if it is connected
	 * 
	 *         check to see if the client specified by the client config is
	 *         connected or not
	 */
	@Override
	public boolean isClientConnected(String clientId) {
		boolean connected = false;
		kernel.GetAllConnectionInfo();
		for (int i = 0; i < kernel.GetNumberConnections(); ++i) {
			ConnectionInfo info = kernel.GetConnectionInfo(i);
			if (info.GetName().equalsIgnoreCase(clientId)) {
				connected = true;
				break;
			}
		}
		return connected;
	}

	/**
	 * @param agentName
	 *            tailor the command to this agent name
	 * @return a string command line to execute to spawn the debugger
	 */
	public String getDebuggerCommand(String agentName) {
		// Figure out whether to use java or javaw
		String os = System.getProperty("os.name");
		String commandLine;
		if (os.matches(".+indows.*") || os.matches("INDOWS")) {
			commandLine = "javaw -jar \""
					+ SimConfig.getHome()
					+ File.separator
					+ "..\\..\\SoarLibrary\\bin\\SoarJavaDebugger.jar\" -cascade -remote -agent "
					+ agentName + " -port " + port;
		} else {
			commandLine = System.getProperty("java.home")
					+ "/bin/java -jar "
					+ SimConfig.getHome()
					+ File.separator
					+ "../../SoarLibrary/bin/SoarJavaDebugger.jar -XstartOnFirstThread -cascade -remote -agent "
					+ agentName + " -port " + port;
		}

		return commandLine;
	}

	@Override
	public void reload(String player) {
		AgentData agentData = agents.get(player);
		if (agentData == null) {
			logger.warn("Didn't find player to reload: " + player);
			return;
		}

		agentData.agent
				.LoadProductions(agentData.productions.getAbsolutePath());
	}

	@Override
	public RobotCommander createRoomCommander(Robot player, RoomWorld world,
			String productions, String[] shutdownCommands) {
		Agent agent = createSoarAgent(player.getName(), productions, debug);
		if (agent == null) {
			return null;
		}
		SoarRobot commander = new SoarRobot(sim, player, agent, kernel, world, shutdownCommands);
		agents.get(player.getName()).sa = commander;
		boolean commitResult = agents.get(player.getName()).agent.Commit();
		if (!commitResult) {
			assert false;
		}
		return commander;
	}

	private static final ExecutorService exec = Executors
			.newSingleThreadExecutor();

	private BlockingQueue<Boolean> inputReady = new SynchronousQueue<Boolean>();
	private BlockingQueue<Boolean> inputProcessed = new SynchronousQueue<Boolean>();
	private BlockingQueue<Boolean> tickReady = new SynchronousQueue<Boolean>();
	private BlockingQueue<Boolean> tickProcessed = new SynchronousQueue<Boolean>();
	private boolean firstUpdate = false;
	
	@Override
	public void onEvent(SimEvent event) {
		try {
			if (event instanceof StartEvent) {
				onStartEvent();
			} else if (event instanceof ResetEvent) {
				updateCount = 0;
			} else {
				if (interrupted.get()) {
					logger.trace("Ignoring event due to interrupted flag");
					return;
				}
				if (event instanceof BeforeTickEvent) {
					onBeforeTickEvent();
				} else if (event instanceof AfterTickEvent) {
					onAfterTickEvent();
				} else if (event instanceof StopEvent) {
					onStopEvent();
				}
			}
		} catch (InterruptedException e) {
			logger.trace("event interrupted");
			interrupted.set(true);
			flushLocks();
		} finally {
			logger.trace("event returning");
		}
	}
	
	private void onStartEvent() {
		firstUpdate = true;
		interrupted.set(false);
		if (agents.size() != 0) {
			exec.submit(new Callable<Void>() {
				@Override
				public Void call() {
					logger.trace("Soar alive");
					for (AgentData ad : agents.values()) {
						boolean commitResult = ad.agent.Commit();
						if (!commitResult) {
							assert false;
						}
					}
					logger.trace("Soar run start");
					logger.trace("Run result: " + kernel.RunAllAgentsForever());
					logger.trace("Soar run stop, stopping sim and flushing locks");
					interrupted.set(true);
					sim.stop();
					flushLocks();
					logger.trace("Soar run returning");
					return null;
				}
			});
		}
	}
	
	private void onBeforeTickEvent() throws InterruptedException {
		if (async) {
			return;
		}
		
        logger.trace("onBeforeTickEvent tickReady");
        tickReady.put(Boolean.TRUE);
        if (interrupted.get()) {
                logger.trace("onBeforeTickEvent interrupted flag is set");
                return;
        }
        logger.trace("onBeforeTickEvent wait for tickProcessed");
        tickProcessed.take();
	}
	
	private void onAfterTickEvent() throws InterruptedException {
		logger.trace("onAfterTickEvent inputReady");
		inputReady.put(Boolean.TRUE);
		if (interrupted.get()) {
			logger.trace("onAfterTickEvent returning due to interrupted true");
			return;
		}
		logger.trace("onAfterTickEvent wait for inputProcessed");
		inputProcessed.take();
	}
	
	private void onStopEvent() throws InterruptedException {
		if (async) {
			logger.trace("StopEvent inputReady");
			inputReady.put(Boolean.FALSE);
		} else {
			logger.trace("StopEvent tickReady");
			tickReady.put(Boolean.FALSE);
		}
	}
	
	private AtomicBoolean interrupted = new AtomicBoolean();
	
	int updateCount = 0;
	@Override
	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) {
		long id = Stopwatch.start("Soar", "updateEventHandler");
		logger.trace("Soar update " + ++updateCount);
		try {
			if (async) {
				asynchronousUpdate();
			} else {
				synchronousUpdate();
			}
		} catch (InterruptedException e) {
			logger.trace("Soar update interrupted");
			interrupted.set(true);
			stop();
			sim.stop();
			flushLocks();
		} finally {
			Stopwatch.stop(id);	
		}
	}
	
	private void asynchronousUpdate() throws InterruptedException {
		Boolean go = inputReady.poll();
		if (go == null) {
			return;
		} else if (go == Boolean.FALSE) {
			logger.trace("Soar update stop");
			stop();
			return;
		}

		for (AgentData ad : agents.values()) {
			logger.trace("Soar update processing output for " + ad.agent.GetAgentName());
			ad.sa.processSoarOuput();
			logger.trace("Soar updating input for " + ad.agent.GetAgentName());
			ad.sa.updateSoarInput();
			boolean commitResult = ad.agent.Commit();
			if (!commitResult) {
				assert false;
			}
		}

		logger.trace("Soar inputProcessed");
		inputProcessed.put(Boolean.TRUE);
	}
	
	private void synchronousUpdate() throws InterruptedException {
		// if stepped through output from debugger and the sim isn't running, 
		// do no synchronization, and don't process output link.
		boolean running = this.sim.isRunning();
		
		if (running) {
	        if (firstUpdate) {
	            logger.trace("Soar synch update first update since start event");
	            firstUpdate = false;
	            tickReady.take();
			}
			
			for (AgentData ad : agents.values()) {
		        logger.trace("Soar synch update processing output for " + ad.agent.GetAgentName());
		        ad.sa.processSoarOuput();
			}
			tickProcessed.put(Boolean.TRUE);
			logger.trace("Soar synch wait inputReady");
			inputReady.take();
		}
		for (AgentData ad : agents.values()) {
		        logger.trace("Soar synch updating input for " + ad.agent.GetAgentName());
		        ad.sa.updateSoarInput();
		        ad.agent.Commit();
		}
		if (running) {
			logger.trace("Soar synch inputProcessed");
			inputProcessed.put(Boolean.TRUE);
			logger.trace("Soar synch wait tickReady");
			Boolean go = tickReady.take();
			if (go == Boolean.FALSE) {
			        logger.trace("Soar synch update sync stop");
			        stop();
			        return;
			}
		}		
	}
	
	private void flushLocks() {
		inputReady.poll();
		inputProcessed.offer(Boolean.FALSE);
	}
	
	private void stop() {
		logger.trace("issuing stop");
		kernel.StopAllAgents();
	}
	
	@Override
	public void setDebug(boolean setting) {
		this.debug = setting;
	}

	boolean async = true;
	@Override
	public boolean isAsync() {
		return async;
	}

	@Override
	public void setAsync(boolean setting) {
		async = setting;
	}

	@Override
	public int getUpdateCount() {
		return updateCount;
	}
}
