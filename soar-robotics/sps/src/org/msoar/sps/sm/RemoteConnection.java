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
import org.msoar.sps.SharedNames;

final class RemoteConnection implements Runnable {
	private static final Logger logger = Logger.getLogger(RemoteConnection.class);
	
	static RemoteConnection newInstance(Socket socket) throws IOException {
		return new RemoteConnection(socket);
	}
	
	private final String component;
	private final PrintWriter out;
	private final BufferedReader in;
	
	private RemoteConnection(Socket socket) throws IOException {
		this.out = new PrintWriter(socket.getOutputStream(), true);
		this.in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
		
		this.component = in.readLine();
		if (this.component == null) {
			throw new IOException();
		}
		
		logger.debug(this.component + " initialized");
	}
	
	String getComponentName() {
		return this.component;
	}
	
	void close() {
		out.println(SharedNames.ServerCommands.CLOSE);
		closeInternal();
	}
	
	private void closeInternal() {
		out.flush();
		out.close();
		try {
			in.close();
		} catch (IOException ignored) {
		}
	}

	public void run() {
		// listen for commands
		try {
			String inputLine;
			while ((inputLine = in.readLine()) != null) {
				SharedNames.ClientCommands command = SharedNames.ClientCommands.valueOf(inputLine);
				if (command == null) {
					// This is not recoverable.
					throw new IOException("Unknown command: " + inputLine);
				}
				
				switch (command) {
				case OUTPUT:
					output();
					break;
					
				case DONE:
					// TODO: done
					break;
					
				case CLOSE:
					closeInternal();
					break;
				}
			}

		} catch (IOException e) {
			close();
			logger.error(e.getMessage());
			e.printStackTrace();
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
			System.out.print(builder.toString());

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
		out.println(SharedNames.ServerCommands.COMMAND);
		out.println(command.size());
		for (String arg : command) {
			out.println(arg);
		}
		out.flush();
	}

	void config(String config) {
		out.println(SharedNames.ServerCommands.CONFIG);
		out.println(config.length());
		out.print(config);
		out.flush();
	}

	void environment(Map<String, String> environment) {
		out.println(SharedNames.ServerCommands.ENVIRONMENT);
		out.println(environment.size());
		for (Entry<String, String> arg : environment.entrySet()) {
			out.println(arg.getKey());
			out.println(arg.getValue());
		}
		out.flush();
	}
	
	void start() {
		out.println(SharedNames.ServerCommands.START);
		out.flush();
	}

	void stop() {
		out.println(SharedNames.ServerCommands.STOP);
		out.flush();
	}
}
