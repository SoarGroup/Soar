package org.msoar.sps.control.io.i;

import java.util.List;

import jmat.LinAlg;
import jmat.MathUtil;
import lcmtypes.pose_t;
import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;

class SelfIL {
	private WaypointsIL waypointsIL;
	private FloatElement xwme;
	private FloatElement ywme;
	private FloatElement zwme;
	private Identifier posewme;
	private IntElement yawwmei;
	private FloatElement yawwmef;
	private long utimeLast = 0;
	private Agent agent;
	private double yawRadians;
	private ReceivedMessagesIL receivedMessagesIL;
	
	SelfIL(Agent agent, Identifier self) {
		this.agent = agent;
		agent.CreateStringWME(self, "name", agent.GetAgentName());

		posewme = agent.CreateIdWME(self, "pose");
		xwme = agent.CreateFloatWME(posewme, "x", 0);
		ywme = agent.CreateFloatWME(posewme, "y", 0);
		zwme = agent.CreateFloatWME(posewme, "z", 0);
		yawRadians = 0;
		yawwmef = agent.CreateFloatWME(posewme, "yaw", 0);
		
		Identifier waypoints = agent.CreateIdWME(self, "waypoints");
		waypointsIL = new WaypointsIL(agent, waypoints);
		
		Identifier receivedwme = agent.CreateIdWME(self, "received-messages");
		receivedMessagesIL = new ReceivedMessagesIL(agent, receivedwme);
	}
	
	private void updateYawWme(boolean useFloatYawWmes) {
		double yawDegrees = Math.toDegrees(yawRadians);
		
		if (useFloatYawWmes) {
			if (yawwmei != null) {
				agent.DestroyWME(yawwmei);
				yawwmei = null;
				yawwmef = agent.CreateFloatWME(posewme, "yaw", 0);
			}
			agent.Update(yawwmef, yawDegrees);
		} else {
			if (yawwmef != null) {
				agent.DestroyWME(yawwmef);
				yawwmef = null;
				yawwmei = agent.CreateIntWME(posewme, "yaw", 0);
			}
			agent.Update(yawwmei, (int)Math.round(yawDegrees));
		}
	}
	
	double getYawRadians() {
		return yawRadians;
	}

	void update(pose_t pose, List<String> tokens, boolean useFloatYawWmes) {
		if (pose == null) {
			return; // no info
		}
		
		if (utimeLast == pose.utime) {
			return; // same info
		}

		agent.Update(xwme, pose.pos[0]);
		agent.Update(ywme, pose.pos[1]);
		agent.Update(zwme, pose.pos[2]);
		yawRadians = LinAlg.quatToRollPitchYaw(pose.orientation)[2];
		yawRadians = MathUtil.mod2pi(yawRadians);
		updateYawWme(useFloatYawWmes);
		
		waypointsIL.update(pose);
		
		// TODO support multiple sources
		if (tokens != null) {
			// TODO tokens len 0 triggering clear is overloaded and messy
			if (tokens.size() == 0) {
				receivedMessagesIL.clear();
			} else {
				receivedMessagesIL.update(tokens);
			}
		}
	}
	
	WaypointsIL getWaypointsIL() {
		return waypointsIL;
	}

	public ReceivedMessagesIL getMessagesIL() {
		return receivedMessagesIL;
	}
}

