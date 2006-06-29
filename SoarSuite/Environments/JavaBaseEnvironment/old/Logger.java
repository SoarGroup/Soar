package utilities;

import java.io.*;
import java.sql.*;
import java.text.DateFormat;

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

		Date d = new Date(System.currentTimeMillis());
		// BUGBUG: cannot get this to work in eclipse on linux on albatros
		//log("Log started: " + DateFormat.getDateInstance().format(d));
	}

	public void log(String message) {
		if (m_Output == null) {
			System.out.println(message);
			return;
		}
		
		try {
			m_Output.write(message);
			m_Output.write(System.getProperty("line.separator"));
			m_Output.flush();
		} catch (IOException e) {
			System.out.println("Warning: logger write failed.");
		}
	}
}
