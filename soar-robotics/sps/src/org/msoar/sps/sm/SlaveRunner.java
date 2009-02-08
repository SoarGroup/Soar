package org.msoar.sps.sm;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;
import java.util.ArrayList;

import org.apache.log4j.Logger;
import org.msoar.sps.Names;

class SlaveRunner {
	private static Logger logger = Logger.getLogger(SlaveRunner.class);
			
	private Runner runner;
	private ObjectInputStream oin;
	private ObjectOutputStream oout;

	SlaveRunner(String component, Socket socket) throws IOException {
		this.oout = new ObjectOutputStream(new BufferedOutputStream(socket.getOutputStream()));
		this.oout.flush();
		this.oin = new ObjectInputStream(new BufferedInputStream(socket.getInputStream()));

		logger.trace("creating local runner");
		this.runner = new LocalRunner(component);
		
		// handshake
		logger.trace("writing component name");
		this.oout.writeObject(component);
		this.oout.flush();
		
		logger.debug("wrote component name");
		if (!readString().equals(Names.NET_OK)) {
			throw new IOException();
		}
		run();
		logger.info("quitting");
	}
	
	private String readString() throws IOException {
		try {
			return (String)oin.readObject();
		} catch (ClassNotFoundException e) {
			logger.error(e.getMessage());
			throw new IOException(e);
		}
	}

	// TODO: how do you check a cast?
	@SuppressWarnings("unchecked")
	private ArrayList<String> readCommand() throws IOException {
		try {
			return (ArrayList<String>)oin.readObject();
		} catch (ClassNotFoundException e) {
			logger.error(e.getMessage());
			throw new IOException(e);
		}
	}

	private void run() throws IOException {
		logger.info("running");
		while (true) {
			String netCommand = readString();
			logger.debug("net command: " + netCommand);

			if (netCommand.equals(Names.NET_CONFIGURE)) {
				runner.configure(readCommand(), readString());
				
			} else if (netCommand.equals(Names.NET_START)) {
				runner.start();
	
			} else if (netCommand.equals(Names.NET_ALIVE)) {
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
}
