package org.msoar.sps.sm;

import java.io.IOException;
import java.net.Socket;

import org.apache.log4j.Logger;

public class SessionSlave {
	private static Logger logger = Logger.getLogger(SessionSlave.class);

	SessionSlave(String component, String hostname) {
		// connect to a SessionManager master at hostname
		logger.info("Connecting as " + component + "@" + hostname + ":" + SessionManager.PORT);
		try {
			Socket socket = new Socket(hostname, SessionManager.PORT);
			logger.info("connected");
			new SlaveRunner(component, socket);
		} catch (IOException e) {
			logger.error(e.getMessage());
			System.exit(1);
		}
		System.exit(0);
	}
}
