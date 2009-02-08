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

public class RemoteRunner implements Runner {
	private static Logger logger = Logger.getLogger(RemoteRunner.class);
	
	private String component;
	private Socket socket;
	private ObjectInputStream oin;
	private ObjectOutputStream oout;

	RemoteRunner(Socket socket) throws IOException {
		this.socket = socket;
		this.oout = new ObjectOutputStream(new BufferedOutputStream(socket.getOutputStream()));
		this.oout.flush();
		this.oin = new ObjectInputStream(new BufferedInputStream(socket.getInputStream()));
		
		logger.debug("reading component name");
		try {
			this.component = (String)oin.readObject();
		} catch (ClassNotFoundException e) {
			logger.error(e.getMessage());
			throw new IOException(e);
		}
		oout.writeObject(Names.NET_OK);
		this.oout.flush();
	}

	@Override
	public void configure(ArrayList<String> command, String config) throws IOException {
		oout.writeObject(Names.NET_CONFIGURE);
		oout.writeObject(command);
		oout.writeObject(config);
		oout.flush();
	}

	@Override
	public void stop() throws IOException {
		oout.writeObject(Names.NET_STOP);
		oout.flush();
	}

	@Override
	public void quit() {
		try {
			oout.writeObject(Names.NET_QUIT);
			oout.flush();
			socket.close();
		} catch (IOException ignored) {
		}
	}

	@Override
	public String getComponentName() {
		return component;
	}

	@Override
	public boolean isAlive() throws IOException {
		oout.writeObject(Names.NET_ALIVE);
		oout.flush();
		Boolean response = false;
		try {
			response = (Boolean)oin.readObject();
		} catch (ClassNotFoundException e) {
			logger.error(e.getMessage());
			throw new IOException(e);
		}
		return response;
	}

	@Override
	public void start() throws IOException {
		oout.writeObject(Names.NET_START);
		oout.flush();
	}

}
