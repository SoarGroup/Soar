package sps.sm;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.List;

import org.apache.log4j.Logger;

import sps.config.Config;

public class Runner {
	private static Logger logger = Logger.getLogger(Runner.class);
	
	private Process process;
	private List<String> command;
	private PrintStream out;
	private File configFile;
	private String component;
	private boolean alive = false;
	
	Runner(String component, ArrayList<String> command, PrintStream out, Config config) {
		if (component == null || command == null || out == null) {
			throw new NullPointerException();
		}
		this.component = component;
		this.command = command;
		this.out = out;
		this.alive = false;

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
	}
	
	String getComponentName() {
		return component;
	}
	
	void start() throws IOException {
		if (process != null) {
			throw new IllegalStateException();
		}
		
		// start the component
		ProcessBuilder builder = new ProcessBuilder(command);
		builder.redirectErrorStream(true); // combine stdout/stderr
		try {
			process = builder.start();
			alive = true;
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
		BufferedReader procIn;
		@Override
		public void run() {
			// pipe this output to 
			procIn = new BufferedReader(new InputStreamReader(process.getInputStream()));
			String line;
			try {
				while((line = procIn.readLine()) != null) {
					out.println(component + ": " + line);
				}
			} catch (IOException e) {
				if (out != null) {
					// we aren't shutting down so this is unexpected
					// don't delete config file for debugging
					logger.error(e.getMessage());
					System.exit(1);
				}
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
			alive = false;
			if (configFile != null) {
				logger.info("Removed temporary file: " + configFile.getAbsolutePath());
				configFile.delete();
				configFile = null;
			}
		}
	}
	
	boolean isAlive() {
		return alive;
	}
	
	void destroy() {
		process.destroy();
	}
}
