package edu.umich.soar.sps.sm;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

import org.apache.log4j.Logger;

final class Acceptor implements Runnable {
	private static final Logger logger = Logger.getLogger(Acceptor.class);

	private final ServerSocket serverSocket;
	private final Components components;
	
	Acceptor(Components components, int port) throws IOException {
		this.serverSocket = new ServerSocket(port);
		this.components = components;

		Thread acceptThread = new Thread(this);
		acceptThread.setDaemon(true);
		acceptThread.start();
	}
	
	public void run() {
		logger.info("Listening on port " + serverSocket.getLocalPort());
		try {
			while (true) {
				Socket clientSocket = serverSocket.accept();
				logger.info("New connection from " + clientSocket.getRemoteSocketAddress());
				this.components.newConnection(clientSocket);
			}
		} catch (IOException e) {
			logger.error(e.getMessage());
			e.printStackTrace();
		}
		logger.error("Listener down");
	}
}

