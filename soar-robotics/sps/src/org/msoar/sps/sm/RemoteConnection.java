package org.msoar.sps.sm;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;

import org.apache.log4j.Logger;
import org.msoar.sps.SharedNames;

final class RemoteConnection {
	private static final Logger logger = Logger.getLogger(RemoteConnection.class);
	
	private final String component;
	private final ObjectOutputStream oout;
	private final ObjectInputStream oin;
	
	RemoteConnection(Socket socket) throws IOException {
		this.oout = new ObjectOutputStream(new BufferedOutputStream(socket.getOutputStream()));
		this.oout.flush();
		
		this.oin = new ObjectInputStream(new BufferedInputStream(socket.getInputStream()));
		
		logger.debug("new remote runner waiting for component name");
		this.component = Runners.readString(oin);
		if (component == null) {
			throw new IOException();
		}

		logger.debug("'" + component + "' received, writing ok");
		oout.writeObject(SharedNames.NET_OK);
		this.oout.flush();
	}
	
	private final static class OutputPump implements Runnable {
		private final BufferedReader output;
		private final String component;
		
		private OutputPump(BufferedReader output, String component) {
			this.output = output;
			this.component = component;
		}
		
		public void run() {
			logger.debug(component + ": output pump alive");
			String out;
			try {
				while (( out = output.readLine()) != null) {
					System.out.println(out);
				}
			} catch (IOException e) {
				logger.error(e.getMessage());
			}
		}
	}
	
	void setOutput(BufferedReader output) {
		Thread thread = new Thread(new OutputPump(output, component));
		thread.setDaemon(true);
		thread.start();
	}
	
	String getComponentName() {
		return component;
	}
	
	ObjectOutputStream getObjectOutputStream() {
		return oout;
	}
	
	ObjectInputStream getObjectInputStream() {
		return oin;
	}
	
	void close() {
		try {
			oout.writeObject(SharedNames.NET_CLOSE);
			oout.flush();
			oout.close();
			oin.close();
		} catch (IOException ignored) {
		}
	}
}
