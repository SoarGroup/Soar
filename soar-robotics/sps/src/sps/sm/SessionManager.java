package sps.sm;

import sps.config.Config;
import sps.config.ConfigFile;

import java.io.IOException;
import java.net.ServerSocket;

import org.apache.log4j.Logger;

public class SessionManager {
	private static Logger logger = Logger.getLogger(SessionManager.class);
	static int PORT = 42140;
	private static String USAGE = "Argument should be config file OR component@hostname";
	
	SessionManager(Config config) {
		logger.info("Listening on port " + PORT);
		try {
			ServerSocket socket = new ServerSocket(PORT);
		} catch (IOException e) {
			logger.error(e.getMessage());
			System.exit(1);
		}
		
		// run simple command interpreter:
		
		// start all
		// start <component>
		
		// stop all
		// stop <component>

		// restart all
		// restart <component>
		
		// quit/exit
	}
	
	public static void main(String[] args) {
		logger.info("Starting up.");
		if (args.length > 0) {
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
				try {
					new SessionManager(new Config(new ConfigFile(args[0])));
				} catch (IOException e) {
					logger.error(e.getMessage());
					logger.error(USAGE);
					System.exit(1);
				}
			}
		}
		System.exit(0);
	}
}
