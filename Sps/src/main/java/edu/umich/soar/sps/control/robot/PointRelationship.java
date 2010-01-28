package edu.umich.soar.sps.control.robot;

import jmat.LinAlg;
import jmat.MathUtil;
import lcmtypes.pose_t;

public class PointRelationship {
	private double distance;
	private double yaw;
	private double relativeBearing;
	
	public double getDistance() {
		return distance;
	}

	public double getYaw() {
		return yaw;
	}

	public double getRelativeBearing() {
		return relativeBearing;
	}

	public static PointRelationship calculate(pose_t self, double [] target) {
		PointRelationship r = new PointRelationship();
		r.distance = LinAlg.distance(self.pos, target);
		double [] delta = LinAlg.subtract(target, self.pos);
		r.yaw = Math.atan2(delta[1], delta[0]);
		r.relativeBearing = r.yaw - LinAlg.quatToRollPitchYaw(self.orientation)[2];
		r.relativeBearing = MathUtil.mod2pi(r.relativeBearing);
		return r;
	}
}
