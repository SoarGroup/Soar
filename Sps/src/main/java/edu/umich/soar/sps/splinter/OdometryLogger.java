package edu.umich.soar.sps.splinter;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

import org.apache.log4j.Logger;

import edu.umich.soar.sps.lcmtypes.odom_t;

final class OdometryLogger {
	private static final Logger logger = Logger.getLogger(OdometryLogger.class);
	
	private final FileWriter datawriter;
	
	OdometryLogger() throws IOException {
		File datafile = File.createTempFile("odom-", ".txt", new File(System.getProperty("user.dir")));
		datawriter = new FileWriter(datafile);
		logger.info("Opened " + datafile.getAbsolutePath());
	}
	
	void record(odom_t now) throws IOException {
		if (now == null) {
			logger.debug("record called with null odometry reading");
			return;
		}
		
		// record
		datawriter.append(Long.toString(now.utime));
		datawriter.append(",");
		datawriter.append(Integer.toString(now.left));
		datawriter.append(",");
		datawriter.append(Integer.toString(now.right));
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