package org.msoar.sps.sm;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;
import java.util.List;
import java.util.Map;

import org.apache.log4j.Logger;
import org.msoar.sps.SharedNames;

final class SlaveRunner {
	private static final Logger logger = Logger.getLogger(SlaveRunner.class);
			
	private final Runner runner;
	private final ObjectInputStream oin;
	private final ObjectOutputStream oout;

	SlaveRunner(String component, Socket controlSocket, Socket outputSocket) throws IOException {
		try {
			this.oout = new ObjectOutputStream(new BufferedOutputStream(controlSocket.getOutputStream()));
			this.oout.flush();
			
			this.oin = new ObjectInputStream(new BufferedInputStream(controlSocket.getInputStream()));
	
			logger.trace("creating local runner");
			this.runner = LocalRunner.newSlaveInstance(component, outputSocket);
			
			// handshake
			logger.trace("writing component name");
			this.oout.writeObject(component);
			this.oout.flush();
			
			logger.debug("wrote component name");
			if (!NetworkRunner.readString(oin).equals(SharedNames.NET_OK)) {
				throw new IOException();
			}
			
			run();
			logger.info("quitting");
		} catch (IOException e) {
			throw e;
		} finally {
			quit();
		}
	}
	
	private void run() throws IOException {
		logger.info("running");
		while (true) {
			String netCommand = NetworkRunner.readString(oin);
			logger.debug("net command: " + netCommand);

			if (netCommand.equals(SharedNames.NET_CONFIGURE)) {
				List<String> command = NetworkRunner.readCommand(oin);
				String config = null;
				Map<String, String> environment = null;
				
				netCommand = NetworkRunner.readString(oin);
				if (netCommand.equals(SharedNames.NET_CONFIG_YES)) {
					config = NetworkRunner.readString(oin);
				} else {
					if (!netCommand.equals(SharedNames.NET_CONFIG_NO)) {
						throw new IOException("didn't get config yes/no message");
					}
				}

				netCommand = NetworkRunner.readString(oin);
				if (netCommand.equals(SharedNames.NET_ENVIRONMENT_YES)) {
					environment = NetworkRunner.readEnvironment(oin);
				} else {
					if (!netCommand.equals(SharedNames.NET_ENVIRONMENT_NO)) {
						throw new IOException("didn't get environment yes/no message");
					}
				}

				runner.configure(command, config, environment);
				
			} else if (netCommand.equals(SharedNames.NET_START)) {
				runner.start();
	
			} else if (netCommand.equals(SharedNames.NET_ALIVE)) {
				if (runner.isAlive()) {
					this.oout.writeObject(new Boolean(true));
				} else {
					this.oout.writeObject(new Boolean(false));
				}
				oout.flush();
				
			} else if (netCommand.equals(SharedNames.NET_STOP)) {
				runner.stop();
	
			} else if (netCommand.equals(SharedNames.NET_QUIT)) {
				return;
			}
		}
	}
	
	void quit() {
		try {
			if (runner != null) {
				runner.quit();
			}
			if (oin != null) {
				oin.close();
			}
			if (oout != null) {
				oout.close();
			}
		} catch (IOException ignored) {
		}
	}
}
