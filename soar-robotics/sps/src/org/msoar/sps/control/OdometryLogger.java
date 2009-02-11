package org.msoar.sps.control;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

import org.apache.log4j.Logger;
import org.msoar.sps.Names;
import org.msoar.sps.lcmtypes.odom_t;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;

class OdometryLogger implements LCMSubscriber {
	private static Logger logger = Logger.getLogger(OdometryLogger.class);
	
	private odom_t odom;
	private odom_t prev;
	private FileWriter datawriter;
	
	OdometryLogger() throws IOException {
		LCM lcm = LCM.getSingleton();
		lcm.subscribe(Names.ODOM_CHANNEL, this);
		
		File datafile = File.createTempFile("odom-", ".txt", new File(System.getProperty("user.dir")));
		datawriter = new FileWriter(datafile);
		logger.info("Opened " + datafile.getAbsolutePath());
	}
	
	void update(boolean tag) throws IOException {
		record(odom); 
		
		if (tag) {
			logger.info("mark");
			datawriter.append("\n");
			datawriter.flush();
		}
	}
	
	private void record(odom_t now) throws IOException {
		if (now == null) {
			logger.debug("record called with null odometry reading");
			return;
		}
		if (prev == null || now.utime > prev.utime) {
			if (prev == null || now.left != prev.left || now.right != prev.right) {
				// record
				datawriter.append(Long.toString(now.utime));
				datawriter.append(",");
				datawriter.append(Integer.toString(now.left));
				datawriter.append(",");
				datawriter.append(Integer.toString(now.right));
				datawriter.append("\n");
				datawriter.flush();
				
				if (prev == null) {
					prev = new odom_t();
				}
				prev.left = now.left;
				prev.right = now.right;
			}
			prev.utime = now.utime;
		}
	}
	
	void close() {
		LCM lcm = LCM.getSingleton();
		lcm.unsubscribe(Names.ODOM_CHANNEL, this);
		try {
			datawriter.close();
		} catch (IOException e) {
			logger.error("Error closing stream" + e.getMessage());
		}
		logger.info("closed odometry file");
	}

	@Override
	public void messageReceived(LCM lcm, String channel, DataInputStream ins) {
		if (channel.equals(Names.ODOM_CHANNEL)) {
			try {
				odom = new odom_t(ins);
			} catch (IOException e) {
				logger.error("Error decoding odom_t message: " + e.getMessage());
			}
		}
	}
}
