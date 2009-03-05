package org.msoar.sps.sm;

import java.io.IOException;
import java.net.Socket;

import org.apache.log4j.Logger;
import org.msoar.sps.Names;

final class SessionSlave {
	private static final Logger logger = Logger.getLogger(SessionSlave.class);
	
	SessionSlave(String component, String hostname, int port) {
		// connect to a SessionManager master at hostname
		logger.info("Connecting as " + component + "@" + hostname + ":" + port);
		try {
			Socket controlSocket = new Socket(hostname, port);
			controlSocket.getOutputStream().write(Names.TYPE_COMPONENT);
			controlSocket.getOutputStream().flush();
			
			Socket outputSocket = new Socket(hostname, port);
			outputSocket.getOutputStream().write(Names.TYPE_OUTPUT);
			outputSocket.getOutputStream().flush();
			
			logger.info("connected");
			new SlaveRunner(component, controlSocket, outputSocket);

		} catch (IOException e) {
			logger.error(e.getMessage());
		}
	}
}
