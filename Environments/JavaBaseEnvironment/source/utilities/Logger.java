package utilities;

import java.io.*;
import java.sql.*;

public class Logger {
	public static Logger logger = new Logger();
	
	private Writer m_Output;

	public void toFile(String filename, boolean append) {
		if (filename == null) {
			System.out.println("Log filename null.");
			return;
		}
		
		try {
			m_Output = new BufferedWriter(new FileWriter(new File(filename), append));
		} catch (IOException e) {
			System.out.println("Exception creating logger: " + e.getMessage());
			System.exit(1);
		}

		Timestamp ts = new Timestamp(System.currentTimeMillis());
		log("Log started: " + ts);
	}

	public void log(String message) {
		if (m_Output == null) {
			System.out.println(message);
			return;
		}
		
		try {
			m_Output.write(message);
			m_Output.write("\n");
			m_Output.flush();
		} catch (IOException e) {
			System.out.println("Warning: logger write failed.");
		}
	}
}
