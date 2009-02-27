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

import org.apache.log4j.Logger;

public class LocalRunner implements Runner {
	private static Logger logger = Logger.getLogger(LocalRunner.class);
	
	// constant
	private String component;
	private PrintStream out;
	
	// can change per run
	private List<String> command;
	private String config;

	// will change per run
	private Process process;
	private File configFile;
	
	LocalRunner(String component) {
		setComponent(component);
	}
	
	LocalRunner(String component, Socket outputSocket) throws IOException {
		setComponent(component);
		if (outputSocket == null) {
			throw new NullPointerException();
		}

		logger.debug("setting up output socket");
		this.out = new PrintStream(outputSocket.getOutputStream());
		this.out.print(component + "\r\n");
		this.out.flush();
	}
	
	private void setComponent(String component) {
		if (component == null) {
			throw new NullPointerException();
		}
		this.component = component;
	}
	
	public String getComponentName() {
		return component;
	}
	
	public void configure(ArrayList<String> command, String config) {
		if (command == null) {
			throw new NullPointerException();
		}
		this.command = command;
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
	
	class OutputHandler implements Runnable {
		private BufferedReader procIn;
		public void run() {
			// pipe this output to 
			procIn = new BufferedReader(new InputStreamReader(process.getInputStream()));
			String line;
			try {
				while((line = procIn.readLine()) != null) {
					if (out == null) {
						System.out.println(component + ": " + line);
					} else {
						out.println(component + ": " + line);
						out.flush();
					}
				}
			} catch (IOException e) {
				logger.warn(e.getMessage());
			}
		}
	}
	
	class AliveHandler implements Runnable {
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
