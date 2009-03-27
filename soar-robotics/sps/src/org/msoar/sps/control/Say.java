package org.msoar.sps.control;

import java.io.IOException;
import java.io.PrintWriter;

final class Say {
	static void newMessage(String message) {
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
