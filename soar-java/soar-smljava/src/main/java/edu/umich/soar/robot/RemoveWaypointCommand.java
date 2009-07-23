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
 * Remove waypoint from waypoint system.
 */
final public class RemoveWaypointCommand extends NoDDCAdapter implements Command {
	private static final Logger logger = Logger.getLogger(RemoveWaypointCommand.class);
	static final String NAME = "remove-waypoint";

	static Command newInstance(WaypointInterface waypoints) {
		return new RemoveWaypointCommand(waypoints);
	}
	
	public RemoveWaypointCommand(WaypointInterface waypoints) {
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

		logger.debug(String.format("%s: %16s", NAME, id));

		if (waypoints.removeWaypoint(id) == false) {
			logger.warn(NAME + ": Unable to remove waypoint " + id + ", no such waypoint");
			CommandStatus.error.addStatus(agent, command);
			return false;
		}

		CommandStatus.accepted.addStatus(agent, command);
		CommandStatus.complete.addStatus(agent, command);
		return true;
	}
}
