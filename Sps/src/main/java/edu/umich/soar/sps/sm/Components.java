package edu.umich.soar.sps.sm;

import java.io.IOException;
import java.net.Socket;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import org.apache.log4j.Logger;
import edu.umich.soar.config.Config;

final class Components implements DoneListener {
	private static final Logger logger = Logger.getLogger(Components.class);

	private static final class Names {
		private static final String DOT_COMMAND = ".command";
		private static final String DOT_CONFIG = ".config";
		private static final String DOT_ENVIRONMENT = ".environment";

		private Names() { throw new AssertionError(); }
	}
	
	private final Map<String, RemoteConnection> connections = new ConcurrentHashMap<String, RemoteConnection>();
	private final Map<String, Runner> running = new ConcurrentHashMap<String, Runner>();
	private final Config config;
	
	Components(Config config) {
		this.config = config;
	}
	
	void newConnection(Socket socket) {
		try {
			RemoteConnection rc = RemoteConnection.newInstance(socket, this);
			assert rc != null;
			
			if (connections.containsKey(rc.getComponentName())) {
				logger.warn("Replacing old connection for " + rc.getComponentName());
				connections.remove(rc.getComponentName()).close();
			}
			
			connections.put(rc.getComponentName(), rc);
			
		} catch (IOException e) {
			logger.error(e.getMessage());
			e.printStackTrace();
		}
	}
	
	private void startAll() {
		String[] components = config.getStrings("all");
		if (components != null) {
			for (String component : components) {
				assert component != null;
				start(component);
			}
		}
	}
	
	void start(String component) {
		if (component == null) {
			startAll();
			return;
		}
		logger.info("Start " + component);
		
		String[] command = config.getStrings(component + Names.DOT_COMMAND);
		if (command == null) {
			logger.error("No command for component: " + component);
			return;
		}
		
		if (running.containsKey(component)) {
			logger.error("Already running: " + component);
			return;
		} 
		
		try {
			RemoteConnection rc = connections.get(component);
			Runner runner;
			if (rc == null) {
				logger.debug("Creating new local runner for " + component);
				runner = LocalRunner.newInstance(component, Arrays.asList(command), getConfigString(component), getEnvironmentString(component), this);
			} else {
				logger.debug("Creating new remote runner for " + component);
				runner = RemoteRunner.newInstance(rc, Arrays.asList(command), getConfigString(component), getEnvironmentString(component));
			}
			logger.info("Created new runner for " + component);
			running.put(component, runner);
		} catch (IOException e) {
			logger.error(e.getMessage());
			e.printStackTrace();
		}
	}
	
	private String getConfigString(String component) {
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

	private void stopAll() {
		for (Runner runner : running.values()) {
			stopInternal(runner);
		}
		running.clear();
	}
	
	void stop(String component) {
		if (component == null) {
			stopAll();
			return;
		}
		logger.info("Stop " + component);
		
		Runner runner = running.remove(component);
		if (runner == null) {
			logger.error("Component not running: " + component);
			return;
		}
		stopInternal(runner);
	}
	
	private void stopInternal(Runner runner) {
		runner.stop();
	}
	
	void close() {
		stopAll();
		for (RemoteConnection rc : connections.values()) {
			rc.close();
		}
		connections.clear();
	}
	
	public void done(String component) {
		logger.info(component + " is done running");
		running.remove(component);
	}
}
