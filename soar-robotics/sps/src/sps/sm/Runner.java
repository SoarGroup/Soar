package sps.sm;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.List;

import org.apache.log4j.Logger;

import sps.config.Config;

public class Runner {
	private static Logger logger = Logger.getLogger(Runner.class);
	
	private Process process;
	private List<String> command;
	private Config config;
	private Connection connection;
	
	Runner(List<String> command, Config config) {
		this.command = command;

		// create temp file
		// write config to temp file
		// add config file to command line
	}
	
	void setConnection(Connection connection) {
		// where to put this component's output
	}
	
	void start() throws IOException {
		// start the component
		ProcessBuilder builder = new ProcessBuilder(command);
		builder.redirectErrorStream(true); // combine stdout/stderr
		process = builder.start();
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
					connection.getOutputStream().write(line);
				}
			} catch (IOException e) {
				logger.error(e.getMessage());
				System.exit(1);
			}
		}
	}
	
	void destroy() {
		process.destroy();
	}
}
