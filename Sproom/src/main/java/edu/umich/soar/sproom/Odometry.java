package edu.umich.soar.sproom;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import april.jmat.LinAlg;
import april.jmat.MathUtil;
import april.lcmtypes.pose_t;

/**
 * Class that translates odometry information to point data.
 *
 * @author voigtjr@gmail.com
 */
public class Odometry {
	private static final Log logger = LogFactory.getLog(Odometry.class);

	private final double tickMeters;
	private final double baselineMeters;
	private final double[] deltaxyt = new double[3];

	public Odometry(double tickMeters, double baselineMeters) {
		this.tickMeters = tickMeters;
		this.baselineMeters = baselineMeters;
	}
	
	public void propagate(OdometryPoint newOdom, OdometryPoint oldOdom, pose_t pose) {
		if (logger.isTraceEnabled()) {
			logger.trace("delta left  ticks: " + (newOdom.getLeft() - oldOdom.getLeft()));
			logger.trace("delta right ticks: " + (newOdom.getRight() - oldOdom.getRight()));
		}
		double dleft = (newOdom.getLeft() - oldOdom.getLeft()) * tickMeters;
		double dright = (newOdom.getRight() - oldOdom.getRight()) * tickMeters;
		
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
					newOdom.getLeft(), newOdom.getRight(), oldOdom.getLeft(), oldOdom.getRight(), 
					pose.pos[0], pose.pos[1], theta));
		}
	}
}
