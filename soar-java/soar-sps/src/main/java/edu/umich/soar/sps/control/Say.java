package edu.umich.soar.sps.control;

import java.io.IOException;
import java.io.PrintWriter;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

import org.apache.log4j.Logger;

final class Say {
	private static final Logger logger = Logger.getLogger(Say.class);

	static void newMessage(String message) {
		if (thread == null) {
			logger.debug("Starting say thread");
			thread = new Thread(new SayThread());
			thread.setDaemon(true);
			thread.start();
		}
		logger.trace("newMessage: " + message);
		messages.add(message);
	}
	
	private static final BlockingQueue<String> messages = new LinkedBlockingQueue<String>();
	private static Thread thread;

	private static class SayThread implements Runnable {
		private SayThread() {
		}
		
		public void run() {
			String message;
			while (true) {
				try {
					message = messages.take();
				} catch (InterruptedException e) {
					e.printStackTrace();
					continue;
				}
				logger.debug("Saying: " + message);
				try {
					ProcessBuilder builder = new ProcessBuilder(new String[] { "/usr/bin/festival", "--tts" } );
					Process process = builder.start();
					PrintWriter writer = new PrintWriter(process.getOutputStream());
					writer.print(message);
					writer.flush();
					writer.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
	}
}
