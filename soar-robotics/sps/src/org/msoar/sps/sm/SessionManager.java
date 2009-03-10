package org.msoar.sps.sm;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import org.apache.log4j.Logger;
import org.msoar.sps.SharedNames;
import org.msoar.sps.config.Config;
import org.msoar.sps.config.ConfigFile;

final class SessionManager implements Runnable {
	private static class Names {
		private static final String USAGE = 
			"Usage: java -jar sps.jar CONFIG_FILE [wait]\n" +
			"       java -jar sps.jar COMPONENT@HOSTNAME\n" +
			"Run session manager master instance using CONFIG_FILE or run slave COMPONENT\n" +
			"on machine HOSTNAME. Master instance starts components locally by default but\n" +
			"can optionally be told to 'wait' for network connections.\n" +
			"\n" +
			"Working directory must be soar-robotics/sps.\n";
		private static final String PROMPT = "sm> ";
		private static final String DOT_COMMAND = ".command";
		private static final String DOT_CONFIG = ".config";
		private static final String DOT_ENVIRONMENT = ".environment";
		private static final String INCLUDE = "include";
	}
	
	private enum Command {
		EXIT, QUIT, START, STOP, RESTART;
	}
	
	private enum CommandResult {
		CONTINUE, STOP;
	}
	
	private static final Logger logger = Logger.getLogger(SessionManager.class);
	private static final int PORT = 42140;
	private final ServerSocket serverSocket;
	private final Map<String, Runner> runners = new HashMap<String, Runner>();
	private final Config config;
	
	private SessionManager(boolean autoStart, Config config) {
		if (config == null) {
			throw new NullPointerException();
		}
		this.config = config;
		
		logger.info("Starting up.");
		ServerSocket serverSocket = null;
		try {
			serverSocket = new ServerSocket(PORT);
		} catch (IOException e) {
			logger.error(e.getMessage());
			System.exit(1);
		}
		this.serverSocket = serverSocket;
		
		Thread acceptThread = new Thread(this);
		acceptThread.setDaemon(true);
		acceptThread.start();
		
		if (autoStart) {
			start(null);
		}
		
		// run simple command interpreter:
		BufferedReader input = new BufferedReader(new InputStreamReader(System.in));
		String command = null;
		try {
			System.out.print(Names.PROMPT);
			while ((command = input.readLine()) != null) {
				if (handleCommand(command) == CommandResult.STOP) {
					break;
				}
				System.out.print(Names.PROMPT);
			}
		} catch (IOException e) {
			logger.error(e.getMessage());
		}
	}

	public void run() {
		logger.info("Listening on port " + serverSocket.getLocalPort());
		while (true) {
			try {
				Socket clientSocket = serverSocket.accept();
				logger.info("New connection from " + clientSocket.getRemoteSocketAddress());
				
				byte[] t = new byte[1];
				int read = 0;
				while (read == 0) {
					read = clientSocket.getInputStream().read(t, 0, 1);
				}
				
				if (SharedNames.TYPE_COMPONENT.equals(t[0])) {
					logger.debug("Connection is component");
					Runner runner = new RemoteRunner(clientSocket);
					runners.put(runner.getComponentName(), runner);
					logger.info("Created new remote runner for " + runner.getComponentName());

				} else if (SharedNames.TYPE_OUTPUT.equals(t[0])) {
					logger.debug("Connection is output");
					BufferedReader br = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
					logger.debug("Waiting for name");
					String component = br.readLine();
					logger.debug("Got name: " + component);
					runners.get(component).setOutput(br);
					logger.info("Associated output stream with remote runner " + component);

				} else {
					logger.error("Unknown type header: " + Byte.toString(t[0]));
				}
			} catch (IOException e) {
				logger.error("New connection failed: " + e.getMessage());
				continue;
			}
		}
	}
	
	private CommandResult handleCommand(String command) {
		logger.trace("command: " + command);
		String[] args = command.split(" "); // TODO: try " +"
		if (args[0].length() != 0) {
			Command cmd = Command.valueOf(args[0].toUpperCase());
			if (cmd == null) {
				logger.error("Unknown command: " + args[0]);
			} else {
				String component = args.length > 1 ? args[1] : null;
				switch (cmd) {
				case QUIT:
				case EXIT:
					stopAll();
					quitAll();
					return CommandResult.STOP;
					
				case START:
					start(component);
					break;
					
				case STOP:
					stop(component);
					break;
					
				case RESTART:
					stop(component);
					try {
						logger.info("Sleeping for 5 seconds.");
						Thread.sleep(5000);
					} catch (InterruptedException ignored) {
					}
					start(component);
					break;
				}
			}
		}
		return CommandResult.CONTINUE;
	}
	
	private void start(String component) {
		if (component == null) {
			startAll();
			return;
		}
		logger.info("Start " + component);
		
		String[] command = config.getStrings(component + Names.DOT_COMMAND);
		if (command == null) {
			logger.error("Unknown component: " + component);
			
			if (runners.containsKey(component)) {
				logger.error("Pruning component " + component);
				runners.remove(component);
			}
			return;
		}
		
		Runner runner = runners.get(component);
		if (runner != null) {
			try {
				if (runner.isAlive()) {
					logger.error("Already running: " + component);
					return;
				}
			} catch (IOException e) {
				logger.error("Component error: " + e.getMessage());
				logger.error("Pruning component " + component);
				runners.remove(component);
			}
		} else {
			logger.info("Creating new local runner for " + component);
			runner = LocalRunner.newInstance(component);
			runners.put(component, runner);
		}
		
		try {
			runner.configure(Arrays.asList(command), getConfigString(component), getEnvironmentString(component));
			runner.start();
		} catch (IOException e) {
			logger.error(e.getMessage());
		}
	}
	
	private String getConfigString(String component){
		StringBuilder sb = new StringBuilder();
		boolean hadConfig = false;
		for (String componentConfigId : config.getStrings(component + Names.DOT_CONFIG, new String[0])) {
			hadConfig = true;
			sb.append(config.getChild(componentConfigId).toString());
			sb.append("\n");
		}
		if (hadConfig) {
			return sb.toString();
		}
		return null;
	}
	
	private Map<String, String> getEnvironmentString(String component) {
		Map<String, String> theNewEnvironment = new HashMap<String, String>();
		boolean hadEnvironment = false;
		for (String componentEnvironmentId : config.getStrings(component + Names.DOT_ENVIRONMENT, new String[0])) {
			hadEnvironment = true;
			Config childConf = config.getChild(componentEnvironmentId);
			String[] keys = childConf.getKeys();
			for (String key : keys) {
				theNewEnvironment.put(key, childConf.getString(key));
			}
		}
		if (hadEnvironment) {
			return theNewEnvironment;
		}
		return null;
	}

	private void startAll() {
		String[] components = config.getStrings("all");
		for (String component : components) {
			start(component);
		}
	}
	
	private void stopAll() {
		Set<String> components = runners.keySet();
		for (String component : components) {
			stop(component);
		}
	}
	
	private void quitAll() {
		for (Runner runner : runners.values()) {
			runner.quit();
		}
	}
	
	private void stop(String component) {
		if (component == null) {
			stopAll();
			return;
		}
		logger.info("Stop " + component);
		
		Runner runner = runners.get(component);
		if (runner != null) {
			try {
				if (runner.isAlive()) {
					runner.stop();
					return;
				}
			} catch (IOException e) {
				logger.error("Component error: " + e.getMessage());
				logger.error("Pruning component " + component);
				runners.remove(component);
			}
		}
		logger.error("Component not running: " + component);
	}
	
	public static void main(String[] args) {
		if (args.length > 2 || args.length < 1) {
			System.err.print(Names.USAGE);
			System.exit(1);
		}

		if (args[0].indexOf("@") != -1) {
			if (args.length == 2) {
				System.err.print(Names.USAGE);
				System.exit(1);
			}
			// start a slave
			String[] netInfo = args[0].split("@");
			if (netInfo.length != 2) {
				System.err.print(Names.USAGE);
				System.exit(1);
			}
			new SessionSlave(netInfo[0], netInfo[1], PORT);
			
		} else {
			// load config
			Config config = null;
			try {
				config = new Config(new ConfigFile(args[0]));
				for (String include : config.getStrings(Names.INCLUDE, new String[0])) {
					Config includedConfig = new Config(new ConfigFile("config" + File.separator + include));
					config.merge(includedConfig);
				}
			} catch (IOException e) {
				System.err.println(e.getMessage());
				System.err.print(Names.USAGE);
				System.exit(1);
			}
			
			boolean autoStart = true;
			if (args.length == 2) {
				if (args[1].equals("wait")) {
					autoStart = false;
				}
			}
			new SessionManager(autoStart, config);
		}
	}
}
