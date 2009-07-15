package org.msoar.sps.sm;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.log4j.Logger;
import org.msoar.sps.SharedNames.ClientCommands;
import org.msoar.sps.SharedNames.ServerCommands;

final class RemoteConnection implements Runnable {
	private static final Logger logger = Logger.getLogger(RemoteConnection.class);
	
	static RemoteConnection newInstance(Socket socket, DoneListener done) throws IOException {
		return new RemoteConnection(socket, done);
	}
	
	private final String component;
	private final PrintWriter out;
	private final BufferedReader in;
	private final DoneListener done;
	
	private RemoteConnection(Socket socket, DoneListener done) throws IOException {
		this.out = new PrintWriter(socket.getOutputStream(), true);
		this.in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
		this.done = done;
		
		this.component = in.readLine();
		if (this.component == null) {
			throw new IOException();
		}
		
		Thread thread = new Thread(this);
		thread.start();
		
		logger.debug(this.component + " initialized");
	}
	
	String getComponentName() {
		return this.component;
	}
	
	void close() {
		out.println(ServerCommands.CLOSE);
	}
	
	public void run() {
		// listen for commands
		try {
			String inputLine;
			while ((inputLine = in.readLine()) != null) {
				ClientCommands command = ClientCommands.valueOf(inputLine);
				
				boolean close = false;
				switch (command) {
				case OUTPUT:
					output();
					break;
					
				case DONE:
					done.done(this.component);
					break;
					
				case CLOSE:
					out.println(ServerCommands.CLOSE);
					close = true;
					break;
				}
				if (close) {
					logger.trace("closed, shutting down receive loop");
					break;
				}
			}

		} catch (IOException e) {
			logger.error(e.getMessage());
			e.printStackTrace();
		} finally {
			out.close();
			try {
				in.close();
			} catch (IOException ignored) {}
		}
	}
	
	private void output() throws IOException {
		try {
			String inputLine = in.readLine();
			if (inputLine == null) {
				logger.error("no argument on output command");
				throw new IOException();
			}

			int total = Integer.valueOf(inputLine);
			
			char[] cbuf = new char[total];
			int sofar = 0;
			while (sofar < total) {
				int read = in.read(cbuf, sofar, total - sofar);
				if (read < 0) {
					logger.error("error reading all of output");
					throw new IOException();
				}
				sofar += read;
			}
			StringBuilder builder = new StringBuilder();
			builder.append(this.component);
			builder.append(": ");
			builder.append(cbuf);
			System.out.println(builder.toString());

		} catch (NumberFormatException e) {
			logger.error("malformed argument on output command");
			throw new IOException(e);
		}
	}
	
	@Override
	public int hashCode() {
		return component.hashCode();
	}

	void command(List<String> command) {
		out.println(ServerCommands.COMMAND);
		out.println(command.size());
		for (String arg : command) {
			out.println(arg);
		}
	}

	void config(String config) {
		out.println(ServerCommands.CONFIG);
		out.println(config.length());
		out.write(config);
		out.flush();
	}

	void environment(Map<String, String> environment) {
		out.println(ServerCommands.ENVIRONMENT);
		out.println(environment.size());
		for (Entry<String, String> arg : environment.entrySet()) {
			out.println(arg.getKey());
			out.println(arg.getValue());
		}
	}
	
	void start() {
		out.println(ServerCommands.START);
	}

	void stop() {
		out.println(ServerCommands.STOP);
	}
}
