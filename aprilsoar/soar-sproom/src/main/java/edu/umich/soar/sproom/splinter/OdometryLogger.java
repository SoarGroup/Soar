package edu.umich.soar.sproom.splinter;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.OdometryPoint;

final class OdometryLogger {
	private static final Log logger = LogFactory.getLog(OdometryLogger.class);
	
	private final FileWriter datawriter;
	
	OdometryLogger() throws IOException {
		File datafile = File.createTempFile("odom-", ".txt", new File(System.getProperty("user.dir")));
		datawriter = new FileWriter(datafile);
		logger.info("Opened " + datafile.getAbsolutePath());
	}
	
	void record(OdometryPoint now) throws IOException {
		if (now == null) {
			logger.debug("record called with null odometry reading");
			return;
		}
		
		// record
		datawriter.append(now.toString());
		datawriter.append("\n");
		datawriter.flush();
	}
	
	void close() {
		try {
			datawriter.close();
			logger.info("closed odometry file");
		} catch (IOException e) {
			logger.error("Error closing stream" + e.getMessage());
		}
	}

}