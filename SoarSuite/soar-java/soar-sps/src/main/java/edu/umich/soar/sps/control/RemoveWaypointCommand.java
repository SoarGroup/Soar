/**
 * 
 */
package edu.umich.soar.sps.control;

import org.apache.log4j.Logger;

import edu.umich.soar.waypoints.OffsetPose;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 *
 * Remove waypoint from waypoint system.
 */
final class RemoveWaypointCommand extends NoDDCAdapter implements Command {
	private static final Logger logger = Logger.getLogger(RemoveWaypointCommand.class);
	static final String NAME = "remove-waypoint";

	public boolean execute(InputLinkInterface inputLink, Agent agent,
			Identifier command, OffsetPose opose,
			OutputLinkManager outputLinkManager) {

		String id = command.GetParameterValue("id");
		if (id == null) {
			logger.warn(NAME + ": No id on command");
			CommandStatus.error.addStatus(agent, command);
			return false;
		}

		logger.debug(String.format("%s: %16s", NAME, id));

		if (inputLink.removeWaypoint(id) == false) {
			logger.warn(NAME + ": Unable to remove waypoint " + id + ", no such waypoint");
			CommandStatus.error.addStatus(agent, command);
			return false;
		}

		CommandStatus.accepted.addStatus(agent, command);
		CommandStatus.complete.addStatus(agent, command);
		return true;
	}
}
