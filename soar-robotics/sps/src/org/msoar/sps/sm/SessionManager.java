package org.msoar.sps.sm;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import org.apache.log4j.Logger;
import org.msoar.sps.config.Config;
import org.msoar.sps.config.ConfigFile;

public class SessionManager implements Runnable {
	private static Logger logger = Logger.getLogger(SessionManager.class);
	static int PORT = 42140;
	private static String USAGE = "Argument should be config file OR component@hostname\n\n *** WORKING DIRECTORY MUST BE soar-robotics/sps ***";
	private ServerSocket serverSocket;
	private Map<String, Runner> runners = new HashMap<String, Runner>();
	private Config config;
	
	SessionManager(Config config) {
		String test = "test\n";
		test = test.trim();
		System.out.print("test: '" + test + "'");
		
		if (config == null) {
			throw new NullPointerException();
		}
		
		this.config = config;
		
		try {
			serverSocket = new ServerSocket(PORT);
		} catch (IOException e) {
			logger.error(e.getMessage());
			System.exit(1);
		}
		
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

	@Override
	public void run() {
		logger.info("Listening on port " + serverSocket.getLocalPort());
		while (true) {
			try {
				Socket clientSocket = serverSocket.accept();
				logger.info("New connection from " + clientSocket.getRemoteSocketAddress());

				logger.debug("Creating remote runner");
				Runner runner = new RemoteRunner(clientSocket);
				logger.info("Created new remote runner for " + runner.getComponentName());
				runners.put(runner.getComponentName(), runner);

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
			runner = new LocalRunner(component);
			runners.put(component, runner);
		}
		
		Config componentConfig = null;
		String componentConfigId = config.getString(component + ".config");
		if (componentConfigId != null) {
			componentConfig = config.getChild(componentConfigId);
		}

		try {
			runner.configure(new ArrayList<String>(Arrays.asList(command)), componentConfig == null ? null : componentConfig.toString());
			runner.start();
		} catch (IOException e) {
			logger.error(e.getMessage());
		}
	}
	
	private void stopAll() {
		ArrayList<String> components = new ArrayList<String>(runners.keySet());
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
			new SessionSlave(netInfo[0], netInfo[1]);
			
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
