package org.msoar.sps.sm;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.PrintStream;
import java.net.Socket;
import java.util.List;
import java.util.Map;

import org.apache.log4j.Logger;
import org.msoar.sps.SharedNames;

final class ClientConnection {
	private static final Logger logger = Logger.getLogger(ClientConnection.class);
	
	static ClientConnection newInstance(String component, String hostname, int port) throws IOException {
		return new ClientConnection(component, hostname, port);
	}

	private final ObjectInputStream oin;
	private final ObjectOutputStream oout;

	private ClientConnection(String component, String hostname, int port) throws IOException {
		// connect to a SessionManager master at hostname
		logger.info("Connecting as " + component + "@" + hostname + ":" + port);

		Socket controlSocket = new Socket(hostname, port);
		controlSocket.getOutputStream().write(SharedNames.TYPE_COMPONENT);
		controlSocket.getOutputStream().flush();
		
		Socket outputSocket = new Socket(hostname, port);
		outputSocket.getOutputStream().write(SharedNames.TYPE_OUTPUT);
		outputSocket.getOutputStream().flush();
		
		logger.info("connected");
		oout = new ObjectOutputStream(new BufferedOutputStream(controlSocket.getOutputStream()));
		oout.flush();
		
		oin = new ObjectInputStream(new BufferedInputStream(controlSocket.getInputStream()));

		logger.debug("setting up output socket");
		PrintStream out = new PrintStream(outputSocket.getOutputStream());
		out.print(component + "\r\n");
		out.flush();
		
		// handshake
		logger.trace("writing component name");
		oout.writeObject(component);
		oout.flush();
		
		logger.debug("wrote component name");
		if (!Runners.readString(oin).equals(SharedNames.NET_OK)) {
			throw new IllegalStateException();
		}

		try {
			logger.info("running");
			Runner runner = null;
			while (true) {
				String netCommand = Runners.readString(oin);
				logger.debug("net command: " + netCommand);
	
				if (netCommand.equals(SharedNames.NET_START)) {
					List<String> command = Runners.readCommand(oin);
					String config = null;
					Map<String, String> environment = null;
					
					netCommand = Runners.readString(oin);
					if (netCommand.equals(SharedNames.NET_CONFIG_YES)) {
						config = Runners.readString(oin);
					} else {
						if (!netCommand.equals(SharedNames.NET_CONFIG_NO)) {
							throw new IOException("didn't get config yes/no message");
						}
					}
	
					netCommand = Runners.readString(oin);
					if (netCommand.equals(SharedNames.NET_ENVIRONMENT_YES)) {
						environment = Runners.readEnvironment(oin);
					} else {
						if (!netCommand.equals(SharedNames.NET_ENVIRONMENT_NO)) {
							throw new IOException("didn't get environment yes/no message");
						}
					}
	
					logger.trace("creating local runner");
					runner = LocalRunner.newSlaveInstance(component, out, command, config, environment);

				} else if (netCommand.equals(SharedNames.NET_STOP)) {
					if (runner == null) {
						throw new IllegalStateException();
					}
					runner.stop();
					runner = null;
		
				} else if (netCommand.equals(SharedNames.NET_CLOSE)) {
					if (runner != null) {
						throw new IllegalStateException();
					}
					if (oin != null) {
						oin.close();
					}
					if (oout != null) {
						oout.close();
					}
					return;
				}
			}
		} catch (IOException e) {
			logger.error(e.getMessage());
			e.printStackTrace();
		}
	}
}
