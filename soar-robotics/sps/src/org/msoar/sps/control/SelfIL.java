package org.msoar.sps.control;

import jmat.LinAlg;
import jmat.MathUtil;
import lcmtypes.pose_t;
import sml.Agent;
import sml.FloatElement;
import sml.Identifier;

class SelfIL {
	private FloatElement xwme;
	private FloatElement ywme;
	private FloatElement zwme;
	private FloatElement yawwme;
	private long utimeLast = 0;
	private Agent agent;
	private double yaw;

	SelfIL(Agent agent, Identifier self) {
		this.agent = agent;
		agent.CreateStringWME(self, "name", agent.GetAgentName());

		Identifier posewme = agent.CreateIdWME(self, "pose");
		xwme = agent.CreateFloatWME(posewme, "x", 0);
		ywme = agent.CreateFloatWME(posewme, "y", 0);
		zwme = agent.CreateFloatWME(posewme, "z", 0);
		yaw = 0;
		yawwme = agent.CreateFloatWME(posewme, "yaw", yaw);
	}
	
	double getYaw() {
		return yaw;
	}

	void update(pose_t pose) {
		if (pose == null) {
			return; // no info
		}
		
		if (utimeLast == pose.utime) {
			return; // same info
		}

		// TODO: possibly need synchronization here
		pose_t poseCopy = pose.copy();
		utimeLast = poseCopy.utime;
		
		agent.Update(xwme, poseCopy.pos[0]);
		agent.Update(ywme, poseCopy.pos[1]);
		agent.Update(zwme, poseCopy.pos[2]);
		yaw = toDisplayYaw(poseCopy.orientation);
		agent.Update(yawwme, yaw);
	}
	
	private double toDisplayYaw(double[] orientation) {
		double newYaw = LinAlg.quatToRollPitchYaw(orientation)[2];
		newYaw = MathUtil.mod2pi(newYaw);
		newYaw = Math.toDegrees(newYaw);
		return newYaw;
	}
}

