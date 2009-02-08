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
import org.msoar.sps.config.Config;

class SlaveRunner {
	private static Logger logger = Logger.getLogger(SlaveRunner.class);
			
	private Runner runner;
	private ObjectInputStream oin;
	private ObjectOutputStream oout;

	SlaveRunner(String component, Socket socket) throws IOException {
		this.oin = new ObjectInputStream(new BufferedInputStream(socket.getInputStream()));
		this.oout = new ObjectOutputStream(new BufferedOutputStream(socket.getOutputStream()));

		this.runner = new LocalRunner(component);
		
		// handshake
		this.oout.writeObject(component);
		this.oout.flush();
		logger.debug("wrote component name");
		if (!readNetCommand().equals(Names.NET_OK)) {
			throw new IOException();
		}
		run();
		logger.info("quitting");
	}
	
	private String readNetCommand() throws IOException {
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

	private Config readConfig() throws IOException {
		try {
			return (Config)oin.readObject();
		} catch (ClassNotFoundException e) {
			logger.error(e.getMessage());
			throw new IOException(e);
		}
	}

	private void run() throws IOException {
		logger.info("running");
		while (true) {
			String netCommand = readNetCommand();
			logger.debug("net command: " + netCommand);

			if (netCommand.equals(Names.NET_CONFIGURE)) {
				runner.configure(readCommand(), readConfig());
				
			} else if (netCommand.equals(Names.NET_START)) {
				runner.start();
	
			} else if (netCommand.equals(Names.NET_ALIVE)) {
				if (runner.isAlive()) {
					this.oout.writeObject(new Boolean(true));
				} else {
					this.oout.writeObject(new Boolean(false));
				}
				
			} else if (netCommand.equals(Names.NET_DESTROY)) {
				runner.destroy();
	
			} else if (netCommand.equals(Names.NET_QUIT)) {
				return;
			}
		}
	}
}
