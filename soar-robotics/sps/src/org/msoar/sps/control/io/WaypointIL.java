package org.msoar.sps.control.io;

import java.util.Arrays;

import org.apache.log4j.Logger;

import jmat.LinAlg;
import jmat.MathUtil;
import lcmtypes.pose_t;
import sml.Agent;
import sml.FloatElement;
import sml.Identifier;

class WaypointIL {
	private static Logger logger = Logger.getLogger(WaypointIL.class);

	private double[] xyz = new double[3];
	private String name;
	private Identifier waypoints;
	private Agent agent;

	private Identifier waypoint;
	private FloatElement absRelativeBearing;
	private FloatElement distance;
	private FloatElement relativeBearing;
	private FloatElement yaw;

	WaypointIL(Agent agent, double[] waypointxyz, String name, Identifier waypoints) {
		this.agent = agent;
		System.arraycopy(waypointxyz, 0, this.xyz, 0, waypointxyz.length);
		this.name = new String(name);
		this.waypoints = waypoints;
	}

	String getName() {
		return name;
	}

	boolean equals(String other) {
		return other.equals(name);
	}

	void update(pose_t pose) {
		double distanceValue = LinAlg.distance(pose.pos, xyz, 2);
		double[] delta = LinAlg.subtract(xyz, pose.pos);
		double yawValueRad = Math.atan2(delta[1], delta[0]);
		double yawValueDeg = Math.toDegrees(yawValueRad);
		
		double relativeBearingValueRad = yawValueRad - LinAlg.quatToRollPitchYaw(pose.orientation)[2];
		relativeBearingValueRad = MathUtil.mod2pi(relativeBearingValueRad);
		double relativeBearingValueDeg = Math.toDegrees(relativeBearingValueRad);
		
		double absRelativeBearingValueDeg = Math.abs(relativeBearingValueDeg);

		if (logger.isTraceEnabled()) {
			logger.trace(String.format("xyz%s y%1.2f rb%1.2f", Arrays.toString(xyz), yawValueRad, relativeBearingValueRad));
		}
		
		if (waypoint == null) {
			waypoint = agent.CreateIdWME(waypoints, "waypoint");
			
			agent.CreateStringWME(waypoint, "id", name);
			agent.CreateFloatWME(waypoint, "x", xyz[0]);
			agent.CreateFloatWME(waypoint, "y", xyz[1]);
			agent.CreateFloatWME(waypoint, "z", xyz[2]);

			distance = agent.CreateFloatWME(waypoint, "distance", distanceValue);
			yaw = agent.CreateFloatWME(waypoint, "yaw", yawValueDeg);
			relativeBearing = agent.CreateFloatWME(waypoint, "relative-bearing", relativeBearingValueDeg);
			absRelativeBearing = agent.CreateFloatWME(waypoint, "abs-relative-bearing", absRelativeBearingValueDeg);
			
		} else {
			agent.Update(distance, distanceValue);
			agent.Update(yaw, yawValueDeg);
			agent.Update(relativeBearing, relativeBearingValueDeg);
			agent.Update(absRelativeBearing, absRelativeBearingValueDeg);
		}
	}

	void disable() {
		agent.DestroyWME(waypoint);
		waypoint = null;
		absRelativeBearing = null;
		distance = null;
		relativeBearing = null;
		yaw = null;
	}
	
	boolean isDisabled() {
		return waypoint == null;
	}
}

