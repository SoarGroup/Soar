package org.msoar.sps.sm;

import java.io.IOException;
import java.net.Socket;

import org.apache.log4j.Logger;
import org.msoar.sps.Names;

public class SessionSlave {
	private static Logger logger = Logger.getLogger(SessionSlave.class);

	SessionSlave(String component, String hostname) {
		// connect to a SessionManager master at hostname
		logger.info("Connecting as " + component + "@" + hostname + ":" + SessionManager.PORT);
		SlaveRunner sr = null;
		try {
			Socket controlSocket = new Socket(hostname, SessionManager.PORT);
			controlSocket.getOutputStream().write(Names.TYPE_COMPONENT);
			controlSocket.getOutputStream().flush();
			
			Socket outputSocket = new Socket(hostname, SessionManager.PORT);
			outputSocket.getOutputStream().write(Names.TYPE_OUTPUT);
			outputSocket.getOutputStream().flush();
			
			logger.info("connected");
			sr = new SlaveRunner(component, controlSocket, outputSocket);

		} catch (IOException e) {
			logger.error(e.getMessage());
		}
		if (sr != null) {
			sr.stop();
		}
	}
}
