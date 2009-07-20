package org.msoar.sps.control;

import java.util.Arrays;

import org.apache.log4j.Logger;

import jmat.LinAlg;
import jmat.MathUtil;
import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;

final class WaypointIL {
	private static final Logger logger = Logger.getLogger(WaypointIL.class);

	private final double[] xyz = new double[3];
	private final String name;
	private final Identifier waypoints;
	private final Agent agent;

	private Identifier waypoint;
	private FloatElement distance;
	private YawWmes yawWmes;
	private double yawValueRad = 0;
	private double relativeBearingValueRad = 0;
	
	WaypointIL(Agent agent, double[] waypointxyz, String name, Identifier waypoints, boolean useFloatWmes) {
		this.agent = agent;
		System.arraycopy(waypointxyz, 0, this.xyz, 0, waypointxyz.length);
		this.name = new String(name);
		this.waypoints = waypoints;
		
		yawWmes = useFloatWmes ? new YawFloatWmes() : new YawIntWmes();
	}

	private interface YawWmes {
		public void create();
		public void update(double yawValueDeg, double relativeBearingValueDeg, double absRelativeBearingValueDeg);
	}
	
	private class YawIntWmes implements YawWmes {
		private IntElement yawWmeI;
		private IntElement relativeBearingWmeI;
		private IntElement absRelativeBearingWmeI;

		public void create() {
			yawWmeI = agent.CreateIntWME(waypoint, "yaw", 0);
			relativeBearingWmeI = agent.CreateIntWME(waypoint, "relative-bearing", 0);
			absRelativeBearingWmeI = agent.CreateIntWME(waypoint, "abs-relative-bearing", 0);
		}
		
		public void update(double yawValueDeg, double relativeBearingValueDeg, double absRelativeBearingValueDeg) {
			agent.Update(yawWmeI, (int)Math.round(yawValueDeg));
			agent.Update(relativeBearingWmeI, (int)Math.round(relativeBearingValueDeg));
			agent.Update(absRelativeBearingWmeI, (int)Math.round(absRelativeBearingValueDeg));
		}
	}

	private class YawFloatWmes implements YawWmes {
		private FloatElement yawWmeF;
		private FloatElement relativeBearingWmeF;
		private FloatElement absRelativeBearingWmeF;

		public void create() {
			yawWmeF = agent.CreateFloatWME(waypoint, "yaw", 0);
			relativeBearingWmeF = agent.CreateFloatWME(waypoint, "relative-bearing", 0);
			absRelativeBearingWmeF = agent.CreateFloatWME(waypoint, "abs-relative-bearing", 0);
		}
		
		public void update(double yawValueDeg, double relativeBearingValueDeg, double absRelativeBearingValueDeg) {
			agent.Update(yawWmeF, yawValueDeg);
			agent.Update(relativeBearingWmeF, relativeBearingValueDeg);
			agent.Update(absRelativeBearingWmeF, absRelativeBearingValueDeg);
		}
	}
	
	String getName() {
		return name;
	}

	boolean equals(String other) {
		return other.equals(name);
	}

	void update(SplinterState splinter) {
		double distanceValue = LinAlg.distance(splinter.getSplinterPose().pos, xyz, 2);
		double[] delta = LinAlg.subtract(xyz, splinter.getSplinterPose().pos);
		yawValueRad = Math.atan2(delta[1], delta[0]);
		relativeBearingValueRad = yawValueRad - LinAlg.quatToRollPitchYaw(splinter.getSplinterPose().orientation)[2];
		relativeBearingValueRad = MathUtil.mod2pi(relativeBearingValueRad);

		if (logger.isTraceEnabled()) {
			logger.trace(String.format("xyz%s y%1.2f rb%1.2f", Arrays.toString(xyz), yawValueRad, relativeBearingValueRad));
		}
		
		if (waypoint == null) {
			waypoint = agent.CreateIdWME(waypoints, "waypoint");
			
			agent.CreateStringWME(waypoint, "id", name);
			agent.CreateFloatWME(waypoint, "x", xyz[0]);
			agent.CreateFloatWME(waypoint, "y", xyz[1]);
			agent.CreateFloatWME(waypoint, "z", xyz[2]);

			distance = agent.CreateFloatWME(waypoint, "distance", 0);

			yawWmes.create();
		}
		
		agent.Update(distance, distanceValue);
		
		double yawValueDeg = Math.toDegrees(yawValueRad);
		double relativeBearingValueDeg = Math.toDegrees(relativeBearingValueRad);
		double absRelativeBearingValueDeg = Math.abs(relativeBearingValueDeg);

		yawWmes.update(yawValueDeg, relativeBearingValueDeg, absRelativeBearingValueDeg);
	}

	void disable() {
		agent.DestroyWME(waypoint);
		waypoint = null;
	}
	
	boolean isDisabled() {
		return waypoint == null;
	}
}

