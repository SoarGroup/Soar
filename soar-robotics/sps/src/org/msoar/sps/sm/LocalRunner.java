package org.msoar.sps.sm;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.Socket;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.log4j.Logger;

final class LocalRunner implements Runner {
	private static final Logger logger = Logger.getLogger(LocalRunner.class);
	
	static LocalRunner newInstance(String component) {
		try {
			return new LocalRunner(component, null);
		} catch (IOException e) {
			//this can't happen
			assert false;
		}
		return null;
	}
	
	static LocalRunner newSlaveInstance(String component, Socket outputSocket) throws IOException {
		if (outputSocket == null) {
			throw new NullPointerException();
		}
		return new LocalRunner(component, outputSocket);
	}
	
	// constant
	private final String component;
	private final PrintStream out;
	private final boolean slave;
	
	// can change per run
	private List<String> command;
	private Map<String, String> environment;
	private String config;

	// will change per run
	private Process process;
	private File configFile;
	
	private LocalRunner(String component, Socket outputSocket) throws IOException {
		if (component == null) {
			throw new NullPointerException();
		}
		this.component = component;

		if (outputSocket == null) {
			this.slave = false;
			this.out = System.out;
			return;
		}
		
		this.slave = true;
		logger.debug("setting up output socket");
		this.out = new PrintStream(outputSocket.getOutputStream());
		this.out.print(component + "\r\n");
		this.out.flush();
	}
	
	public String getComponentName() {
		return component;
	}
	
	public void configure(List<String> command, String config, Map<String, String> environment) {
		if (command == null) {
			throw new NullPointerException();
		}
		this.command = new ArrayList<String>(command);
		this.environment = environment;
		this.config = config;
	}
	
	public void start() throws IOException {
		if (process != null) {
			throw new IllegalStateException();
		}
		
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
				FileOutputStream fout = new FileOutputStream(configFile);
				PrintStream out = new PrintStream(fout);
				out.print(config);
			} catch (FileNotFoundException e) {
				throw new IllegalStateException(e);
			}
			
			// add config file to command line
			command.add(configFile.getAbsolutePath());
		}

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
					out.println(component + ": " + line);
					if (slave) {
						out.flush();
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
			
			logger.info(component + " process is dead.");
			process = null;
			if (configFile != null) {
				logger.info("Removed temporary file: " + configFile.getAbsolutePath());
				configFile.delete();
				configFile = null;
			}
		}
	}
	
	public boolean isAlive() {
		return process != null;
	}
	
	public void stop() {
		if (process != null) {
			process.destroy();
		}
	}
	
	public void quit() {
		stop();

		if (out != null) {
			out.close();
		}
	}
	
	public void setOutput(BufferedReader output) {
		logger.error("Called setOutput on local runner");
		assert false;
	}
}
