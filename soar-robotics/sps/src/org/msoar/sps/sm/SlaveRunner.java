package org.msoar.sps.sm;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.PrintStream;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.ArrayList;

import org.apache.log4j.Logger;
import org.msoar.sps.Names;

class SlaveRunner {
	private static Logger logger = Logger.getLogger(SlaveRunner.class);
			
	private Runner runner;
	private ObjectInputStream oin;
	private ObjectOutputStream oout;
	private PrintStream out;

	SlaveRunner(String component, Socket controlSocket, Socket outputSocket) throws IOException {
		try {
			this.oout = new ObjectOutputStream(new BufferedOutputStream(controlSocket.getOutputStream()));
			this.oout.flush();
			this.oin = new ObjectInputStream(new BufferedInputStream(controlSocket.getInputStream()));
	
			logger.debug("setting up output socket");
			out = new PrintStream(outputSocket.getOutputStream());
			out.println(component);
			
			logger.trace("creating local runner");
			this.runner = new LocalRunner(component, out);
			
			// handshake
			logger.trace("writing component name");
			this.oout.writeObject(component);
			this.oout.flush();
			
			logger.debug("wrote component name");
			if (!NetworkRunner.readString(oin).equals(Names.NET_OK)) {
				throw new IOException();
			}
			
			run();
			logger.info("quitting");
		} catch (IOException e) {
			throw e;
		} finally {
			stop();
		}
	}
	
	private void run() throws IOException {
		logger.info("running");
		while (true) {
			String netCommand = NetworkRunner.readString(oin);
			logger.debug("net command: " + netCommand);

			if (netCommand.equals(Names.NET_CONFIGURE)) {
				ArrayList<String> command = NetworkRunner.readCommand(oin);
				netCommand = NetworkRunner.readString(oin);
				if (netCommand.equals(Names.NET_CONFIG_NO)) {
					runner.configure(command, null);
				} else if (netCommand.equals(Names.NET_CONFIG_YES)) {
					runner.configure(command, NetworkRunner.readString(oin));
				} else {
					throw new IOException("didn't get config yes/no message");
				}
				
			} else if (netCommand.equals(Names.NET_START)) {
				runner.start();
	
			} else if (netCommand.equals(Names.NET_ALIVE)) {
				this.oout.writeObject(Names.NET_ALIVE_RESPONSE);
				if (runner.isAlive()) {
					this.oout.writeObject(new Boolean(true));
				} else {
					this.oout.writeObject(new Boolean(false));
				}
				oout.flush();
				
			} else if (netCommand.equals(Names.NET_STOP)) {
				runner.stop();
	
			} else if (netCommand.equals(Names.NET_QUIT)) {
				return;
			}
		}
	}
	
	void stop() {
		if (runner != null) {
			try {
				runner.stop();
			} catch (IOException ignored) {
			}
		}
	}
}
