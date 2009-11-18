package edu.umich.soar.sps.sm;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

import org.apache.log4j.Logger;

final class CommandLineInterface {
	private static final Logger logger = Logger.getLogger(CommandLineInterface.class);

	private static class Names {
		private static final String PROMPT = "sm> ";
		
		private Names() { throw new AssertionError(); }
	}

	private enum Command { EXIT, QUIT, START, STOP, RESTART; }
	private enum CommandResult { CONTINUE, STOP; }
	
	private final Components components;

	CommandLineInterface(Components components) {
		this.components = components;
	}
	
	void run(boolean autoStart) throws IOException {
		if (autoStart) {
			components.start(null);
		}
		
		BufferedReader input = new BufferedReader(new InputStreamReader(System.in));
		String command = null;
		System.out.print(Names.PROMPT);
		while ((command = input.readLine()) != null) {
			if (handleCommand(command) == CommandResult.STOP) {
				break;
			}
			System.out.print(Names.PROMPT);
		}
	}
	
	private CommandResult handleCommand(String command) {
		logger.trace("command: " + command);
		String[] args = command.split(" "); // TODO: try " +"
		if (args[0].length() != 0) {
			Command cmd = null;
			try {
				cmd = Command.valueOf(args[0].toUpperCase());
			} catch (IllegalArgumentException ignored) {
			}
			if (cmd == null) {
				logger.error("Unknown command: " + args[0]);
			} else {
				String component = args.length > 1 ? args[1] : null;
				switch (cmd) {
				case QUIT:
				case EXIT:
					components.close();
					return CommandResult.STOP;
					
				case START:
					components.start(component);
					break;
					
				case STOP:
					components.stop(component);
					break;
					
				case RESTART:
					components.stop(component);
					try {
						logger.info("Sleeping for 5 seconds.");
						Thread.sleep(5000);
					} catch (InterruptedException ignored) {
					}
					components.start(component);
					break;
				}
			}
		}
		return CommandResult.CONTINUE;
	}
}

