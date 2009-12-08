package edu.umich.soar.sproom.control;

import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.SharedNames;

import lcm.lcm.LCM;
import lcm.lcm.LCMDataInputStream;
import lcm.lcm.LCMSubscriber;
import lcmtypes.differential_drive_command_t;
import lcmtypes.pose_t;

final class LCMProxy implements LCMSubscriber {
	private static final Log logger = LogFactory.getLog(LCMProxy.class);

	static final LCMProxy getInstance() {
		// TODO: enforce singleton? maybe?
		return new LCMProxy();
	}
	
	private final LCM lcm = LCM.getSingleton();
	private pose_t lcmPose;
	
	private LCMProxy() {
		lcm.subscribe(SharedNames.POSE_CHANNEL, this);
	}

	void transmitDC(differential_drive_command_t dc) {
		if (logger.isTraceEnabled()) {
			logger.trace("transmit: " + dc.left + "," + dc.right);
		}
		lcm.publish(SharedNames.DRIVE_CHANNEL, dc);
	}
	
	pose_t getPose() {
		return lcmPose;
	}
	
	public void messageReceived(LCM lcm, String channel, LCMDataInputStream ins) {
		if (channel.equals(SharedNames.POSE_CHANNEL)) {
			try {
				lcmPose = new pose_t(ins);
			} catch (IOException e) {
				logger.error("Error decoding pose_t message: " + e.getMessage());
			}
		}
	}
}
