package org.msoar.sps.sm;


import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
//import java.net.ServerSocket;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import org.apache.log4j.Logger;
import org.msoar.sps.config.Config;
import org.msoar.sps.config.ConfigFile;

public class SessionManager {
	private static Logger logger = Logger.getLogger(SessionManager.class);
	static int PORT = 42140;
	private static String USAGE = "Argument should be config file OR component@hostname";
	//private ServerSocket socket;
	private Map<String, Runner> runners = new HashMap<String, Runner>();
	private Config config;
	
	SessionManager(Config config) {
		if (config == null) {
			throw new NullPointerException();
		}
		
		this.config = config;
//		try {
//			socket = new ServerSocket(PORT);
//			logger.info("Listening on port " + PORT);
//		} catch (IOException e) {
//			logger.error(e.getMessage());
//			System.exit(1);
//		}
		
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
			System.exit(1);
		}
	}
	
	private boolean handleCommand(String command) {
		String[] args = command.split(" ");
		
		if (args[0].length() != 0) {
			// quit/exit
			if (args[0].equals("quit") || args[0].equals("exit")) {
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
				start(args.length > 1 ? args[1] : null);
				
			} else {
				logger.error("Unknown command: " + args[0]);
			}
		}
		return true;
	}
	
	private void start(String component) {
		if (component == "all") {
			String[] components = config.getStrings("all");
			for (String c : components) {
				if (c.equals("all")) {
					assert false;
					logger.error("Skipping 'all' component");
				} else {
					start(c);
				}
			}
		}
		
		logger.info("Start " + component);
		
		String[] command = config.getStrings(component + ".command");
		if (command == null) {
			logger.error("Unknown component: " + component);
			return;
		}
		
		Runner runner = runners.get(component);
		if (runner != null) {
			if (runner.isAlive()) {
				logger.error("Already running: " + component);
				return;
			}
			logger.trace("Pruning dead component: " + runner.getComponentName());
			runners.remove(component);
		}
		runner = null;
		
		Config componentConfig = null;
		String componentConfigId = config.getString(component + ".config");
		if (componentConfigId != null) {
			componentConfig = config.getChild(component + "." + componentConfigId);
		}
		
		try {
			runner = new Runner(component, new ArrayList<String>(Arrays.asList(command)), System.out, componentConfig);
			runner.start();
			runners.put(component, runner);
		} catch (IOException e) {
			logger.error(e.getMessage());
		}
	}
	
	private void stop(String component) {
		if (component == "all") {
			String[] components = config.getStrings("all");
			for (String c : components) {
				if (c.equals("all")) {
					assert false;
					logger.error("Skipping 'all' component");
				} else {
					stop(c);
				}
			}
		}
		
		logger.info("Stop " + component);
		
		Runner runner = runners.remove(component);
		if (runner != null) {
			if (runner.isAlive()) {
				runner.destroy();
				return;
			}
			logger.trace("Pruning dead component: " + runner.getComponentName());
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
			} catch (IOException e) {
				logger.error(e.getMessage());
				logger.error(USAGE);
				System.exit(1);
			}
			new SessionManager(config);
		}
		System.exit(0);
	}
}
