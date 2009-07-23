/**
 * 
 */
package edu.umich.soar.robot;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 *
 * Enable waypoint.
 */
final public class EnableWaypointCommand extends NoDDCAdapter implements Command {
	private static final Logger logger = Logger.getLogger(EnableWaypointCommand.class);
	static final String NAME = "enable-waypoint";

	static Command newInstance(WaypointInterface waypoints) {
		return new EnableWaypointCommand(waypoints);
	}
	
	public EnableWaypointCommand(WaypointInterface waypoints) {
		this.waypoints = waypoints;
	}

	private final WaypointInterface waypoints;

	@Override
	public boolean execute(Agent agent, Identifier command) {
		String id = command.GetParameterValue("id");
		if (id == null) {
			logger.warn(NAME + ": No id on command");
			CommandStatus.error.addStatus(agent, command);
			return false;
		}

		logger.debug(String.format(NAME + ": %16s", id));

		if (waypoints.enableWaypoint(id) == false) {
			logger.warn(NAME + ": Unable to enable waypoint " + id + ", no such waypoint");
			CommandStatus.error.addStatus(agent, command);
			return false;
		}

		CommandStatus.accepted.addStatus(agent, command);
		CommandStatus.complete.addStatus(agent, command);
		return true;
	}
}
