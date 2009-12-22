package edu.umich.soar.sproom.command;

import java.io.IOException;

import jmat.LinAlg;
import jmat.MathUtil;

import lcm.lcm.LCM;
import lcm.lcm.LCMDataInputStream;
import lcm.lcm.LCMSubscriber;
import lcmtypes.pose_t;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.SharedNames;


public class Pose implements LCMSubscriber {
	private static final Log logger = LogFactory.getLog(Pose.class);

	private final LCM lcm = LCM.getSingleton();
	private pose_t pose;
	private pose_t elaboratedPose = new pose_t();
	
	Pose() {
		lcm.subscribe(SharedNames.POSE_CHANNEL, this);
	}
	
	public pose_t getPose() {
		pose_t temp = pose; // grab a reference once
		if (temp != null && temp.utime > elaboratedPose.utime) {
			// elaborate
			if (elaboratedPose.utime != 0) {
				double dt = (temp.utime - elaboratedPose.utime) * (1.0 / 1000000.0); // (new usec - old usec) * 1 / usec in sec

				temp.vel[0] = (temp.pos[0] - elaboratedPose.pos[0]) * (1.0 / dt);	// TODO: may need modification for smoothing
				temp.vel[1] = (temp.pos[1] - elaboratedPose.pos[1]) * (1.0 / dt);
				
				double newTheta = LinAlg.quatToRollPitchYaw(temp.orientation)[2];
				newTheta = MathUtil.mod2pi(newTheta);
				double oldTheta = LinAlg.quatToRollPitchYaw(elaboratedPose.orientation)[2];
				oldTheta = MathUtil.mod2pi(oldTheta);
				temp.rotation_rate[2] = MathUtil.mod2pi(newTheta - oldTheta) * (1.0 / dt);
				
				if (logger.isTraceEnabled()) {
					double xvel = LinAlg.rotate2(temp.vel, -newTheta)[0];
					logger.trace(String.format("dt%1.5f vx%1.3f vy%1.3f r%1.3f xv%1.3f", dt, temp.vel[0], temp.vel[1], temp.rotation_rate[2], xvel));
				}
			}
			elaboratedPose = temp;
		}
		return elaboratedPose.copy();
	}

	public void messageReceived(LCM lcm, String channel, LCMDataInputStream ins) {
		if (channel.equals(SharedNames.POSE_CHANNEL)) {
			try {
				pose = new pose_t(ins);
			} catch (IOException e) {
				logger.error("Error decoding pose_t message: " + e.getMessage());
			}
		}
	}

}
