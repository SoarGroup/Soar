package org.msoar.sps.sm;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.log4j.Logger;

final class LocalRunner implements Runner {
	private static final Logger logger = Logger.getLogger(LocalRunner.class);
	
	static LocalRunner newInstance(String component, List<String> command, String config, Map<String, String> environment, DoneListener done) throws IOException {
		return new LocalRunner(component, null, command, config, environment, done);
	}
	
	static LocalRunner newSlaveInstance(String component, ClientConnection client, List<String> command, String config, Map<String, String> environment, DoneListener done) throws IOException {
		return new LocalRunner(component, client, command, config, environment, done);
	}
	
	private final String component;
	private final Process process;
	private final File configFile;
	private final ClientConnection client;
	private final DoneListener done;
	
	private LocalRunner(String component, ClientConnection client, List<String> command, String config, Map<String, String> environment, DoneListener done) throws IOException {
		if (component == null) {
			throw new NullPointerException();
		}
		this.component = component;

		if (command == null) {
			throw new NullPointerException();
		}
		command = new ArrayList<String>(command);

		this.client = client;
		this.done = done;
		
		File configFile = null;
		if (config != null) {
			// create temp file
			try {
				configFile = File.createTempFile(component + "-", ".sps", new File(System.getProperty("user.dir")));
			} catch (IOException e) {
				logger.error("Could not create temporary file for configuration!");
				throw new IllegalStateException(e);
			}
			
			logger.info("Created temporary file: " + configFile.getAbsolutePath());
			
			// write config to temp file
			try {
				new PrintStream(new FileOutputStream(configFile)).print(config);
			} catch (FileNotFoundException e) {
				throw new IllegalStateException(e);
			}
			
			// add config file to command line
			command.add(configFile.getAbsolutePath());
		}
		this.configFile = configFile;

		// start the component
		ProcessBuilder builder = new ProcessBuilder(command);
		builder.redirectErrorStream(true); // combine stdout/stderr
		
		// pass addition environment variables if requested
		logger.debug("checking for environment");
		if (environment != null) {
			builder.environment().putAll(environment);
			
			if (logger.isDebugEnabled()) {
				for (Entry<String, String> entry : environment.entrySet()) {
					logger.debug("adding to environment: " + entry.getKey() + " => " + entry.getValue());
				}
			}
		}
		
		try {
			logger.info("Starting process " + component);
			process = builder.start();
		} catch (IOException e) {
			if (configFile != null) {
				configFile.delete();
			}
			throw e;
		}
		
		Thread outputHandler = new Thread(new OutputHandler());
		outputHandler.start();
		Thread aliveHandler = new Thread(new AliveHandler());
		aliveHandler.start();
	}
	
	public String getComponentName() {
		return component;
	}
	
	private final class OutputHandler implements Runnable {
		private final BufferedReader procIn;
		
		private OutputHandler() {
			// pipe this output to 
			procIn = new BufferedReader(new InputStreamReader(process.getInputStream()));
		}
		
		public void run() {
			try {
				String line;
				while((line = procIn.readLine()) != null) {
					if (client == null) {
						System.out.println(line);
					} else {
						client.output(line);
					}
				}
			} catch (IOException e) {
				logger.warn(e.getMessage());
			}
		}
	}
	
	private final class AliveHandler implements Runnable {
		public void run() {
			while (true) {
				try {
					process.waitFor();
				} catch (InterruptedException ignored) {
					logger.warn(component + " interrupted");
				}

				try {
					process.exitValue();
					
					// it's really dead
					break;
				} catch (IllegalStateException ignored) {
					logger.warn(component + " still alive");
				}
			}
			
			logger.debug(component + " process is dead.");
			if (configFile != null) {
				logger.info("Removed temporary file: " + configFile.getAbsolutePath());
				configFile.delete();
			}
			done.done(component);
		}
	}
	
	public void stop() {
		process.destroy();
	}
}
