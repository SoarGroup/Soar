package org.msoar.sps.sm;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

import org.apache.log4j.Logger;
import org.msoar.sps.config.Config;


public class LocalRunner implements Runner {
	private static Logger logger = Logger.getLogger(LocalRunner.class);
	
	// constant
	private String component;

	// can change per run
	private List<String> command;
	private Config config;

	// will change per run
	private Process process;
	private File configFile;
	
	LocalRunner(String component) {
		if (component == null) {
			throw new NullPointerException();
		}
		this.component = component;
	}
	
	@Override
	public String getComponentName() {
		return component;
	}
	
	@Override
	public void configure(ArrayList<String> command, Config config) {
		if (command == null) {
			throw new NullPointerException();
		}
		this.command = command;
		this.config = config;
	}
	
	@Override
	public void start() throws IOException {
		if (process != null) {
			throw new IllegalStateException();
		}
		
		if (config != null) {
			// create temp file
			try {
				configFile = File.createTempFile("sps-", ".config", new File(System.getProperty("user.dir")));
			} catch (IOException e) {
				logger.error("Could not create temporary file for configuration!");
				throw new IllegalStateException(e);
			}
			
			logger.info("Created temporary file: " + configFile.getAbsolutePath());
			
			// write config to temp file
			try {
				config.save(configFile.getAbsolutePath());
			} catch (FileNotFoundException e) {
				throw new IllegalStateException(e);
			}
			
			// add config file to command line
			command.add(configFile.getAbsolutePath());
		}

		// start the component
		ProcessBuilder builder = new ProcessBuilder(command);
		builder.redirectErrorStream(true); // combine stdout/stderr
		try {
			process = builder.start();
		} catch (IOException e) {
			configFile.delete();
			throw e;
		}
		Thread outputHandler = new Thread(new OutputHandler());
		outputHandler.start();
		Thread aliveHandler = new Thread(new AliveHandler());
		aliveHandler.start();
	}
	
	class OutputHandler implements Runnable {
		private BufferedReader procIn;
		@Override
		public void run() {
			// pipe this output to 
			procIn = new BufferedReader(new InputStreamReader(process.getInputStream()));
			String line;
			try {
				while((line = procIn.readLine()) != null) {
					System.out.println(component + ": " + line);
				}
			} catch (IOException e) {
				logger.warn(e.getMessage());
			}
		}
	}
	
	class AliveHandler implements Runnable {
		@Override
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
	
	@Override
	public boolean isAlive() {
		return process != null;
	}
	
	@Override
	public void destroy() {
		process.destroy();
	}
}
