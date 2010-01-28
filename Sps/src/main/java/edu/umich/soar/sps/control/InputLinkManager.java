package edu.umich.soar.sps.control;

import org.apache.log4j.Logger;

import edu.umich.soar.sps.control.robot.ConfigurationIL;
import edu.umich.soar.sps.control.robot.ReceiveMessagesInterface;
import edu.umich.soar.sps.control.robot.ConfigureInterface;
import edu.umich.soar.sps.control.robot.OffsetPose;
import edu.umich.soar.sps.control.robot.TimeIL;
import edu.umich.soar.sps.control.robot.WaypointInterface;

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

	private final ConfigurationIL configurationIL;
	private final TimeIL timeIL;
	private final SelfIL selfIL;
	private final RangerIL rangerIL;

	InputLinkManager(Agent agent, Kernel kernel, int rangesCount, OffsetPose opose) {
		logger.debug("Initializing input-link");

		Identifier inputLink = agent.GetInputLink();

		Identifier configuration = agent.CreateIdWME(inputLink, "configuration");
		configurationIL = new ConfigurationIL(configuration, opose);

		Identifier time = agent.CreateIdWME(inputLink, "time");
		timeIL = new TimeIL(time);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, timeIL, null);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, timeIL, null);

		Identifier self = agent.CreateIdWME(inputLink, "self");
		selfIL = new SelfIL(agent, self, opose, configurationIL);

		Identifier ranges = agent.CreateIdWME(inputLink, "ranges");
		rangerIL = new RangerIL(agent, ranges, rangesCount, configurationIL);
	}

	void update() {
		timeIL.update();
		selfIL.update();
		configurationIL.update();
		rangerIL.update();
	}
	
	WaypointInterface getWaypointInterface() {
		return selfIL.getWaypointsIL();
	}

	ReceiveMessagesInterface getReceiveMessagesInterface() {
		return selfIL.getMessagesIL();
	}
	
	ConfigureInterface getConfigurationInterface() {
		return configurationIL;
	}
}
