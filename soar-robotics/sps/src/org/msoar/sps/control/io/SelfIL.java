package org.msoar.sps.control.io;

import jmat.LinAlg;
import jmat.MathUtil;
import lcmtypes.pose_t;
import sml.Agent;
import sml.FloatElement;
import sml.Identifier;

class SelfIL {
	private WaypointsIL waypointsIL;
	private FloatElement xwme;
	private FloatElement ywme;
	private FloatElement zwme;
	private FloatElement yawwme;
	private long utimeLast = 0;
	private Agent agent;
	private double yawRadians;

	SelfIL(Agent agent, Identifier self) {
		this.agent = agent;
		agent.CreateStringWME(self, "name", agent.GetAgentName());

		Identifier posewme = agent.CreateIdWME(self, "pose");
		xwme = agent.CreateFloatWME(posewme, "x", 0);
		ywme = agent.CreateFloatWME(posewme, "y", 0);
		zwme = agent.CreateFloatWME(posewme, "z", 0);
		yawRadians = 0;
		yawwme = agent.CreateFloatWME(posewme, "yaw", 0);
		
		Identifier waypoints = agent.CreateIdWME(self, "waypoints");
		waypointsIL = new WaypointsIL(agent, waypoints);
	}
	
	double getYawRadians() {
		return yawRadians;
	}

	void update(pose_t pose) {
		if (pose == null) {
			return; // no info
		}
		
		if (utimeLast == pose.utime) {
			return; // same info
		}

		utimeLast = pose.utime;
		
		agent.Update(xwme, pose.pos[0]);
		agent.Update(ywme, pose.pos[1]);
		agent.Update(zwme, pose.pos[2]);
		yawRadians = LinAlg.quatToRollPitchYaw(pose.orientation)[2];
		yawRadians = MathUtil.mod2pi(yawRadians);
		agent.Update(yawwme, Math.toDegrees(yawRadians));
		
		waypointsIL.update(pose);
	}
	
	WaypointsIL getWaypointsIL() {
		return waypointsIL;
	}
}

