package org.msoar.sps.sm;

import java.io.IOException;
import java.net.Socket;

import org.apache.log4j.Logger;

public class SessionSlave {
	private static Logger logger = Logger.getLogger(SessionSlave.class);

	SessionSlave(String component, String hostname) {
		// connect to a SessionManager master at hostname
		logger.info("Connecting as " + component + "@" + hostname + ":" + SessionManager.PORT);
		SlaveRunner sr = null;
		try {
			Socket socket = new Socket(hostname, SessionManager.PORT);
			logger.info("connected");
			sr = new SlaveRunner(component, socket);
		} catch (IOException e) {
			logger.error(e.getMessage());
		}
		if (sr != null) {
			sr.stop();
		}
	}
}
