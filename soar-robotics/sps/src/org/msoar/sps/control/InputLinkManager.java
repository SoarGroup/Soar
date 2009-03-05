package org.msoar.sps.control;

import java.util.List;

import org.apache.log4j.Logger;

import lcmtypes.laser_t;
import lcmtypes.pose_t;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 * Soar input link management. Also handles some updating of waypoint state.
 */
final class InputLinkManager {
	private static final Logger logger = Logger.getLogger(InputLinkManager.class);

	private final Agent agent;
	private final TimeIL timeIL;
	private final SelfIL selfIL;
	private final RangerIL rangerIL;

	InputLinkManager(Agent agent, int rangesCount) {
		this.agent = agent;
		this.agent.SetBlinkIfNoChange(false);

		logger.debug("Initializing input-link");

		Identifier inputLink = agent.GetInputLink();

		Identifier time = agent.CreateIdWME(inputLink, "time");
		timeIL = new TimeIL(agent, time);

		Identifier self = agent.CreateIdWME(inputLink, "self");
		selfIL = new SelfIL(agent, self);

		Identifier ranges = agent.CreateIdWME(inputLink, "ranges");
		rangerIL = new RangerIL(agent, ranges, rangesCount);

		agent.Commit();
	}

	void update(pose_t pose, laser_t laser, List<String> tokens, boolean useFloatYawWmes) {
		timeIL.update();
		selfIL.update(pose, tokens, useFloatYawWmes);
		rangerIL.update(laser, useFloatYawWmes);
	}
	
	InputLinkInterface getInterface() {
		return selfIL;
	}
}
