package edu.umich.soar.sps;

import jmat.LinAlg;
import jmat.MathUtil;
import lcmtypes.pose_t;

import org.apache.log4j.Logger;

import edu.umich.soar.sps.lcmtypes.odom_t;

public final class Odometry {
	private static final Logger logger = Logger.getLogger(Odometry.class);

	private final double tickMeters;
	private final double baselineMeters;
	private final double[] deltaxyt = new double[3];

	public Odometry(double tickMeters, double baselineMeters) {
		this.tickMeters = tickMeters;
		this.baselineMeters = baselineMeters;
	}
	
	public void propagate(odom_t newOdom, odom_t oldOdom, pose_t pose) {
		//System.out.println("delta left ticks: " + (newOdom.left - oldOdom.left));
		//System.out.println("delta right ticks: " + (newOdom.right - oldOdom.right));
		double dleft = (newOdom.left - oldOdom.left) * tickMeters;
		double dright = (newOdom.right - oldOdom.right) * tickMeters;
		
		// phi
		deltaxyt[2] = MathUtil.mod2pi((dright - dleft) / baselineMeters);
		
		double dCenter = (dleft + dright) / 2;
		
		// delta x, delta y
		deltaxyt[0] = dCenter * Math.cos(deltaxyt[2]);
		deltaxyt[1] = dCenter * Math.sin(deltaxyt[2]);
		
		// our current theta
		double theta = LinAlg.quatToRollPitchYaw(pose.orientation)[2];
		
		// calculate and store new xyt
		pose.pos[0] += (deltaxyt[0] * Math.cos(theta)) - (deltaxyt[1] * Math.sin(theta));
		pose.pos[1] += (deltaxyt[0] * Math.sin(theta)) + (deltaxyt[1] * Math.cos(theta));
		theta += deltaxyt[2];
		
		// convert theta to quat and store
		pose.orientation = LinAlg.rollPitchYawToQuat(new double[] {0, 0, theta});
		
		if (logger.isTraceEnabled()) {
			theta = MathUtil.mod2pi(theta);
			theta = Math.toDegrees(theta);
			logger.trace(String.format(
					"odom: n(%d,%d) o(%d,%d) p(%5.2f,%5.2f,%5.1f)", 
					newOdom.left, newOdom.right, oldOdom.left, oldOdom.right, 
					pose.pos[0], pose.pos[1], theta));
		}
	}
	
	public void propagate2(odom_t newOdom, odom_t oldOdom, pose_t pose) {
		//System.out.println("delta left ticks: " + (newOdom.left - oldOdom.left));
		//System.out.println("delta right ticks: " + (newOdom.right - oldOdom.right));
		double dleft = (newOdom.left - oldOdom.left) * tickMeters;
		double dright = (newOdom.right - oldOdom.right) * tickMeters;
		
		// phi
		deltaxyt[2] = MathUtil.mod2pi((dright - dleft) / baselineMeters);
		
		double dCenter = (dleft + dright) / 2;
		
		// our current theta
		double theta = LinAlg.quatToRollPitchYaw(pose.orientation)[2];
		
		// delta x, delta y
		deltaxyt[0] = dCenter * Math.cos(theta);
		deltaxyt[1] = dCenter * Math.sin(theta);
		
		// calculate and store new xyt
		pose.pos[0] += deltaxyt[0];
		pose.pos[1] += deltaxyt[1];
		theta += deltaxyt[2];
		
		// convert theta to quat and store
		pose.orientation = LinAlg.rollPitchYawToQuat(new double[] {0, 0, theta});
		
		if (logger.isTraceEnabled()) {
			theta = MathUtil.mod2pi(theta);
			theta = Math.toDegrees(theta);
			logger.trace(String.format(
					"odom: n(%d,%d) o(%d,%d) p(%5.2f,%5.2f,%5.1f)", 
					newOdom.left, newOdom.right, oldOdom.left, oldOdom.right, 
					pose.pos[0], pose.pos[1], theta));
		}
	}
}
