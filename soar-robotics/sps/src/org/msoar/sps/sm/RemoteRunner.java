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

public class RemoteRunner implements Runner {
	private static Logger logger = Logger.getLogger(RemoteRunner.class);
	
	private String component;
	private ObjectInputStream oin;
	private ObjectOutputStream oout;

	RemoteRunner(Socket socket) throws IOException {
		this.oout = new ObjectOutputStream(new BufferedOutputStream(socket.getOutputStream()));
		this.oin = new ObjectInputStream(new BufferedInputStream(socket.getInputStream()));
		
		logger.debug("reading component name");
		try {
			this.component = (String)oin.readObject();
		} catch (ClassNotFoundException e) {
			logger.error(e.getMessage());
			throw new IOException(e);
		}
		oout.writeObject(Names.NET_OK);
	}

	@Override
	public void configure(ArrayList<String> command, Config config) throws IOException {
		oout.writeObject(Names.NET_CONFIGURE);
		oout.writeObject(command);
		oout.writeObject(config);
	}

	@Override
	public void destroy() throws IOException {
		oout.writeObject(Names.NET_DESTROY);
	}

	@Override
	public String getComponentName() {
		return component;
	}

	@Override
	public boolean isAlive() throws IOException {
		oout.writeObject(Names.NET_ALIVE);
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
	}

}
