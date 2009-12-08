package edu.umich.soar.sproom.control;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import sml.Agent;
import sml.Identifier;
import sml.Kernel;
import sml.smlSystemEventId;

/**
 * @author voigtjr
 * Soar input link management. Also handles some updating of waypoint state.
 */
final class InputLinkManager {
	private static final Log logger = LogFactory.getLog(InputLinkManager.class);

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
