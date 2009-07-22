package org.msoar.sps.sm;

import java.io.File;
import java.io.IOException;

import org.apache.log4j.Logger;
import edu.umich.soar.config.Config;
import edu.umich.soar.config.ConfigFile;
import edu.umich.soar.config.ParseError;

final class SessionManager {
	private static class Names {
		private static final String USAGE = 
			"Usage: java -jar sps.jar CONFIG_FILE [wait]\n" +
			"       java -jar sps.jar COMPONENT@HOSTNAME\n" +
			"Run session manager master instance using CONFIG_FILE or run slave COMPONENT\n" +
			"on machine HOSTNAME. Master instance starts components locally by default but\n" +
			"can optionally be told to 'wait' for network connections.\n" +
			"\n" +
			"Working directory must be soar-robotics/sps.\n";
		private static final String INCLUDE = "include";
		
		private Names() { throw new AssertionError(); }
	}
	
	private static final Logger logger = Logger.getLogger(SessionManager.class);
	private static final int PORT = 42140;
	
	private final Components components;
	private final CommandLineInterface cli;
	
	private SessionManager(boolean autoStart, Config config) throws IOException {
		logger.info("Starting up.");

		this.components = new Components(config);
		this.cli = new CommandLineInterface(this.components);

		new Acceptor(this.components, PORT);

		this.cli.run(autoStart);
	}

	public static void main(String[] args) {
		if (args.length > 2 || args.length < 1) {
			System.err.print(Names.USAGE);
			System.exit(1);
		}

		try {
			if (args[0].indexOf("@") != -1) {
				if (args.length == 2) {
					System.err.print(Names.USAGE);
					System.exit(1);
				}
				// start a slave
				String[] netInfo = args[0].split("@");
				if (netInfo.length != 2) {
					System.err.print(Names.USAGE);
					System.exit(1);
				}
					ClientConnection.newInstance(netInfo[0], netInfo[1], PORT);
				
			} else {
				// load config
				Config config = null;
				try {
					config = new Config(new ConfigFile(args[0]));
					for (String include : config.getStrings(Names.INCLUDE, new String[0])) {
						Config includedConfig = new Config(new ConfigFile("config" + File.separator + include));
						config.merge(includedConfig);
					}
				} catch (ParseError e) {
					System.err.println(e.getMessage());
					System.err.print(Names.USAGE);
					System.exit(1);
				}
				
				boolean autoStart = true;
				if (args.length == 2) {
					if (args[1].equals("wait")) {
						autoStart = false;
					}
				}
				new SessionManager(autoStart, config);
			}
			
		} catch (IOException e) {
			System.err.println(e.getMessage());
			System.exit(1);
		}
	}
}
