package edu.umich.soar.sps.control;

import java.util.List;

import org.apache.log4j.Logger;

import edu.umich.soar.robot.OffsetPose;

import sml.Agent;
import sml.Identifier;
import sml.Kernel;
import sml.smlSystemEventId;

/**
 * @author voigtjr
 * Soar input link management. Also handles some updating of waypoint state.
 */
final class InputLinkManager {
	private static final Logger logger = Logger.getLogger(InputLinkManager.class);

	private final Agent agent;
	private final ConfigurationIL configurationIL;
	private final TimeIL timeIL;
	private final SelfIL selfIL;
	private final RangerIL rangerIL;

	InputLinkManager(Agent agent, Kernel kernel, int rangesCount, OffsetPose splinter) {
		this.agent = agent;
		this.agent.SetBlinkIfNoChange(false);

		logger.debug("Initializing input-link");

		Identifier inputLink = agent.GetInputLink();

		Identifier time = agent.CreateIdWME(inputLink, "time");
		timeIL = new TimeIL(agent, time);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, timeIL, null);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, timeIL, null);

		Identifier self = agent.CreateIdWME(inputLink, "self");
		selfIL = new SelfIL(agent, self, splinter);

		Identifier configuration = agent.CreateIdWME(inputLink, "configuration");
		configurationIL = new ConfigurationIL(agent, configuration, splinter);

		Identifier ranges = agent.CreateIdWME(inputLink, "ranges");
		rangerIL = new RangerIL(agent, ranges, rangesCount);

		agent.Commit();
	}

	void update(List<String> tokens, boolean useFloatYawWmes) {
		timeIL.update();
		selfIL.update(tokens, useFloatYawWmes);
		configurationIL.update(useFloatYawWmes);
		rangerIL.update(useFloatYawWmes);
	}
	
	WaypointInterface getInterface() {
		return selfIL;
	}

}
