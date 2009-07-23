/**
 * 
 */
package edu.umich.soar.sps.control;

import org.apache.log4j.Logger;

import edu.umich.soar.robot.OffsetPose;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 *
 * Enable waypoint.
 */
final class EnableWaypointCommand extends NoDDCAdapter implements Command {
	private static final Logger logger = Logger.getLogger(EnableWaypointCommand.class);
	static final String NAME = "enable-waypoint";

	public boolean execute(InputLinkInterface inputLink, Agent agent,
			Identifier command, OffsetPose opose,
			OutputLinkManager outputLinkManager) {
		String id = command.GetParameterValue("id");
		if (id == null) {
			logger.warn(NAME + ": No id on command");
			CommandStatus.error.addStatus(agent, command);
			return false;
		}

		logger.debug(String.format(NAME + ": %16s", id));

		if (inputLink.enableWaypoint(id, opose) == false) {
			logger.warn(NAME + ": Unable to enable waypoint " + id + ", no such waypoint");
			CommandStatus.error.addStatus(agent, command);
			return false;
		}

		CommandStatus.accepted.addStatus(agent, command);
		CommandStatus.complete.addStatus(agent, command);
		return true;
	}
}
