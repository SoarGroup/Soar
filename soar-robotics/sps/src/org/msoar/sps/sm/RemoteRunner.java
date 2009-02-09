package org.msoar.sps.sm;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
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
	private ObjectOutputStream oout;
	private Boolean aliveResponse;
	private ReceiverThread rt;
	
	RemoteRunner(Socket socket) throws IOException {
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
		logger.debug("got component name");
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
			oout.close();
			rt.close();
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
		
		synchronized (rt) {
			try {
				rt.wait();
			} catch (InterruptedException e) {
				throw new IOException(e);
			}
			
			if (aliveResponse == null) {
				throw new IOException();
			}
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
		
		void close() throws IOException {
			this.oin.close();
		}
		
		@Override
		public void run() {
			logger.debug("rt alive");
			try {
				try {
					synchronized (this) {
						component = (String)oin.readObject();
						rt.notify();
					}
				} catch (ClassNotFoundException e) {
					logger.error(e.getMessage());
					return;
				}

				logger.debug("wrote component");

				while(true) {
					String netCommand = NetworkRunner.readString(oin);
					logger.debug("received command: " + netCommand);
					
					if (netCommand.equals(Names.NET_OUTPUT)) {
						System.out.print(NetworkRunner.readString(oin));
					} else if (netCommand.equals(Names.NET_ALIVE_RESPONSE)) {
						synchronized (this) {
							aliveResponse = NetworkRunner.readBoolean(oin);
							rt.notify();
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

	private class OutputPump implements Runnable {
		BufferedReader output;
		
		OutputPump(BufferedReader output) {
			this.output = output;
		}
		
		@Override
		public void run() {
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
	
	@Override
	public void setOutput(BufferedReader output) {
		Thread thread = new Thread(new OutputPump(output));
		thread.setDaemon(true);
		thread.start();
	}
}
