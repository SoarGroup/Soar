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
	private ObjectOutputStream oout;
	private Boolean aliveResponse;
	private ReceiverThread rt;
	
	RemoteRunner(Socket socket) throws IOException {
		this.socket = socket;
		this.oout = new ObjectOutputStream(new BufferedOutputStream(socket.getOutputStream()));
		this.oout.flush();
		
		rt = new ReceiverThread(new ObjectInputStream(new BufferedInputStream(socket.getInputStream())));
		{
			Thread recv = new Thread(rt);
			recv.setDaemon(true);
			recv.start();
		}
		
		logger.debug("reading component name");
		synchronized (rt) {
			try {
				rt.wait();
			} catch (InterruptedException ignored) {
			}
			
			if (component == null) {
				throw new IOException();
			}
		}
		
		oout.writeObject(Names.NET_OK);
		this.oout.flush();
	}

	@Override
	public void configure(ArrayList<String> command, String config) throws IOException {
		oout.writeObject(Names.NET_CONFIGURE);
		oout.writeObject(command);
		if (config == null) {
			oout.writeObject(Names.NET_CONFIG_NO);
		} else {
			oout.writeObject(Names.NET_CONFIG_YES);
			oout.writeObject(config);
		}
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
		aliveResponse = null;
		oout.writeObject(Names.NET_ALIVE);
		oout.flush();
		
		try {
			rt.wait();
		} catch (InterruptedException e) {
			throw new IOException(e);
		}
		
		if (aliveResponse == null) {
			throw new IOException();
		}
		
		return aliveResponse;
	}

	@Override
	public void start() throws IOException {
		oout.writeObject(Names.NET_START);
		oout.flush();
	}

	private class ReceiverThread implements Runnable {
		private ObjectInputStream oin;

		ReceiverThread(ObjectInputStream oin) {
			if (oin == null) {
				throw new NullPointerException();
			}
			this.oin = oin;
		}
		
		@Override
		public void run() {
			try {
				try {
					synchronized (this) {
						component = (String)oin.readObject();
						this.notify();
					}
				} catch (ClassNotFoundException e) {
					logger.error(e.getMessage());
					return;
				}

				while(true) {
					String netCommand = NetworkRunner.readString(oin);
					
					if (netCommand == Names.NET_OUTPUT) {
						System.out.print(NetworkRunner.readString(oin));
					} else if (netCommand == Names.NET_ALIVE_RESPONSE) {
						synchronized (this) {
							aliveResponse = NetworkRunner.readBoolean(oin);
							this.notify();
						}
					} else {
						logger.error("Unknown network command: " + netCommand);
						return;
					}
				}
			} catch (IOException e) {
				logger.error(e.getMessage());
				return;
			}
		}
	}
}
