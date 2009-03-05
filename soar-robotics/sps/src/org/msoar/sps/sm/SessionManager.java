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
import org.msoar.sps.Names;
import org.msoar.sps.config.Config;
import org.msoar.sps.config.ConfigFile;

final class SessionManager implements Runnable {
	private static final Logger logger = Logger.getLogger(SessionManager.class);
	private static final int PORT = 42140;
	private static final String USAGE = 
		"Argument should be config file OR component@hostname\n\n" +
		" *** WORKING DIRECTORY MUST BE soar-robotics/sps ***";
	private final ServerSocket serverSocket;
	private final Map<String, Runner> runners = new HashMap<String, Runner>();
	private final Config config;
	
	private SessionManager(Config config) {
		if (config == null) {
			throw new NullPointerException();
		}
		this.config = config;
		
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
		
		// run simple command interpreter:
		BufferedReader input = new BufferedReader(new InputStreamReader(System.in));
		String command = null;
		try {
			System.out.print("sm> ");
			while ((command = input.readLine()) != null) {
				boolean cont = handleCommand(command);
				if (!cont) {
					break;
				}
				System.out.print("sm> ");
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
				
				if (Names.TYPE_COMPONENT.equals(t[0])) {
					logger.debug("Connection is component");
					Runner runner = new RemoteRunner(clientSocket);
					runners.put(runner.getComponentName(), runner);
					logger.info("Created new remote runner for " + runner.getComponentName());

				} else if (Names.TYPE_OUTPUT.equals(t[0])) {
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
	
	private boolean handleCommand(String command) {
		String[] args = command.split(" ");
		
		if (args[0].length() != 0) {
			// quit/exit
			if (args[0].equals("quit") || args[0].equals("exit")) {
				stopAll();
				quitAll();
				return false;
			} else if (args[0].equals("start")) {
				// start all
				// start <component>
				start(args.length > 1 ? args[1] : null);
				
			} else if (args[0].equals("stop")) {
				// stop all
				// stop <component>
				stop(args.length > 1 ? args[1] : null);
				
			} else if (args[0].equals("restart")) {
				// restart all
				// restart <component>
				stop(args.length > 1 ? args[1] : null);
				try {
					logger.info("Sleeping for 5 seconds.");
					Thread.sleep(5000);
				} catch (InterruptedException ignored) {
				}
				start(args.length > 1 ? args[1] : null);
				
			} else {
				logger.error("Unknown command: " + args[0]);
			}
		}
		return true;
	}
	
	private void start(String component) {
		if (component == null || component == "all") {
			String[] components = config.getStrings("all");
			for (String c : components) {
				if (c.equals("all")) {
					assert false;
					logger.error("Skipping 'all' component");
				} else {
					start(c);
				}
			}
			return;
		}
		
		logger.info("Start " + component);
		
		String[] command = config.getStrings(component + ".command");
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
			runner.configure(Arrays.asList(command), getConfigString(component));
			runner.start();
		} catch (IOException e) {
			logger.error(e.getMessage());
		}
	}
	
	private String getConfigString(String component){
		StringBuilder sb = new StringBuilder();
		boolean hadConfig = false;
		for (String componentConfigId : config.getStrings(component + ".config", new String[0])) {
			hadConfig = true;
			sb.append(config.getChild(componentConfigId).toString());
			sb.append("\n");
		}
		if (hadConfig) {
			return sb.toString();
		}
		return null;
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
		if (component == null || component == "all") {
			String[] components = config.getStrings("all");
			for (String c : components) {
				if (c.equals("all")) {
					assert false;
					logger.error("Skipping 'all' component");
				} else {
					stop(c);
				}
			}
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
		logger.info("Starting up.");
		if (args.length != 1) {
			logger.error(USAGE);
			System.exit(1);
		}

		if (args[0].indexOf("@") != -1) {
			// start a slave
			String[] netInfo = args[0].split("@");
			if (netInfo.length != 2) {
				logger.error(USAGE);
				System.exit(1);
			}
			new SessionSlave(netInfo[0], netInfo[1], PORT);
			
		} else {
			// load config
			Config config = null;
			try {
				config = new Config(new ConfigFile(args[0]));
				for (String include : config.getStrings("include", new String[0])) {
					Config includedConfig = new Config(new ConfigFile("config" + File.separator + include));
					config.merge(includedConfig);
				}
			} catch (IOException e) {
				logger.error(e.getMessage());
				logger.error(USAGE);
				System.exit(1);
			}
			new SessionManager(config);
		}
	}
}
