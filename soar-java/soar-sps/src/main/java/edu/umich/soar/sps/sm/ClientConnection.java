package edu.umich.soar.sps.sm;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.log4j.Logger;

import edu.umich.soar.sps.SharedNames.ClientCommands;
import edu.umich.soar.sps.SharedNames.ServerCommands;

final class ClientConnection implements DoneListener {
	private static final Logger logger = Logger.getLogger(ClientConnection.class);
	
	static ClientConnection newInstance(String component, String hostname, int port) throws IOException {
		return new ClientConnection(component, hostname, port);
	}

	private final PrintWriter out;
	private final BufferedReader in;
	private final String component;

	private ClientConnection(String component, String hostname, int port) throws IOException {
		this.component = component;
		// connect to a SessionManager master at hostname
		logger.info("Connecting as " + component + "@" + hostname + ":" + port);

		Socket socket = new Socket(hostname, port);
		logger.info("connected");

		this.out = new PrintWriter(socket.getOutputStream(), true);
		this.in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
		
		// handshake
		logger.trace("writing component name");
		out.println(component);
		logger.debug("wrote component name");

		// listen for commands
		try {
			String inputLine;
			List<String> commandArgs = null;
			String config = null;
			Map<String, String> environment = null;
			Runner runner = null;
			
			while ((inputLine = in.readLine()) != null) {
				
				ServerCommands command = ServerCommands.valueOf(inputLine);
				
				boolean close = false;
				switch (command) {
				case COMMAND:
					// clear out config and env
					config = null;
					environment = null;
					commandArgs = command();
					break;
					
				case CONFIG:
					config = config();
					break;
					
				case ENVIRONMENT:
					environment = environment();
					break;
					
				case START:
					if (command == null) {
						throw new IllegalStateException();
					}
					if (runner != null) {
						logger.warn("stopping old runner");
						runner.stop();
					}
					logger.trace("creating local runner");
					runner = LocalRunner.newSlaveInstance(component, this, commandArgs, config, environment, this);
					break;
					
				case STOP:
					if (runner == null) {
						logger.warn("already stopped");
					} else {
						runner.stop();
						runner = null;
					}
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
	
	private List<String> command() throws IOException {
		try {
			String inputLine = in.readLine();
			if (inputLine == null) {
				logger.error("no argument on command command");
				throw new IOException();
			}

			int remaining = Integer.valueOf(inputLine);
			List<String> command = new ArrayList<String>(remaining);
			while (remaining > 0) {
				command.add(in.readLine());
				remaining -= 1;
			}
			return command;

		} catch (NumberFormatException e) {
			logger.error("malformed argument on command command");
			throw new IOException(e);
		}
	}
	
	private String config() throws IOException {
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
			return String.valueOf(cbuf);

		} catch (NumberFormatException e) {
			logger.error("malformed argument on output command");
			throw new IOException(e);
		}

	}
	
	private Map<String, String> environment() throws IOException {
		try {
			String inputLine = in.readLine();
			if (inputLine == null) {
				logger.error("no argument on command command");
				throw new IOException();
			}

			int remaining = Integer.valueOf(inputLine);
			Map<String, String> environment = new HashMap<String, String>(remaining);
			while (remaining > 0) {
				String key = in.readLine();
				String value = in.readLine();
				environment.put(key, value);
				remaining -= 1;
			}
			return environment;

		} catch (NumberFormatException e) {
			logger.error("malformed argument on command command");
			throw new IOException(e);
		}
	}
	
	void output(String data) {
		out.println(ClientCommands.OUTPUT);
		out.println(data.length());
		out.write(data);
		out.flush();
	}
	
	public void done(String component) {
		assert this.component.equals(component);
		out.println(ClientCommands.DONE);
	}
}
