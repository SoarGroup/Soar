package edu.umich.soar.sps.control;

import edu.umich.soar.robot.OffsetPose;
import edu.umich.soar.robot.MessagesIL;
import edu.umich.soar.robot.WaypointsIL;
import edu.umich.soar.robot.ConfigureInterface;

import jmat.LinAlg;
import jmat.MathUtil;
import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;

final class SelfIL {
	private final Agent agent;
	private final WaypointsIL waypointsIL;
	private final FloatElement xwme;
	private final FloatElement ywme;
	private final FloatElement zwme;
	private final FloatElement xvelwme;
	private final FloatElement yvelwme;
	private final FloatElement speedwme;
	private final FloatElement yawvelwme;
	private final Identifier posewme;
	private final long utimeLast = 0;
	private final MessagesIL messagesIL;
	private final OffsetPose opose;
	private final ConfigureInterface configure;
	
	private IntElement yawwmei;
	private FloatElement yawwmef;
	
	SelfIL(Agent agent, Identifier self, OffsetPose opose, ConfigureInterface configure) {
		this.agent = agent;
		this.opose = opose;
		this.configure = configure;
		
		agent.CreateStringWME(self, "name", agent.GetAgentName());

		posewme = agent.CreateIdWME(self, "pose");
		xwme = agent.CreateFloatWME(posewme, "x", 0);
		ywme = agent.CreateFloatWME(posewme, "y", 0);
		zwme = agent.CreateFloatWME(posewme, "z", 0);
		yawwmef = agent.CreateFloatWME(posewme, "yaw", 0);

		xvelwme = agent.CreateFloatWME(posewme, "x-velocity", 0);
		yvelwme = agent.CreateFloatWME(posewme, "y-velocity", 0);
		speedwme = agent.CreateFloatWME(posewme, "speed", 0);
		yawvelwme = agent.CreateFloatWME(posewme, "yaw-velocity", 0);
		
		Identifier waypoints = agent.CreateIdWME(self, "waypoints");
		waypointsIL = new WaypointsIL(waypoints, opose, configure);
		
		Identifier receivedwme = agent.CreateIdWME(self, "received-messages");
		messagesIL = new MessagesIL(receivedwme);
	}
	
	private void updateYawWme(boolean useFloatYawWmes, double yawRadians) {
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
	
	void update() {
		if (opose.getPose() == null) {
			return; // no info
		}
		
		if (utimeLast == opose.getPose().utime) {
			return; // same info
		}

		agent.Update(xwme, opose.getPose().pos[0]);
		agent.Update(ywme, opose.getPose().pos[1]);
		agent.Update(zwme, opose.getPose().pos[2]);
		agent.Update(xvelwme, opose.getPose().vel[0]);
		agent.Update(yvelwme, opose.getPose().vel[1]);
		agent.Update(speedwme, LinAlg.magnitude(opose.getPose().vel));

		double yawRadians = LinAlg.quatToRollPitchYaw(opose.getPose().orientation)[2];
		yawRadians = MathUtil.mod2pi(yawRadians);
		updateYawWme(configure.isFloatYawWmes(), yawRadians);
		agent.Update(yawvelwme, Math.toDegrees(opose.getPose().rotation_rate[2]));
		
		waypointsIL.update();
		
		synchronized(messagesIL) {
			messagesIL.update();
		}
	}
	
	WaypointsIL getWaypointsIL() {
		return waypointsIL;
	}

	MessagesIL getMessagesIL() {
		return messagesIL;
	}

}

