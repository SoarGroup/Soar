/**
 * 
 */
package edu.umich.soar.robot;

import org.apache.log4j.Logger;

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
	public boolean execute(Identifier command) {

		String id = command.GetParameterValue("id");
		if (id == null) {
			CommandStatus.error.addStatus(command, NAME + ": No id on command");
			return false;
		}

		logger.debug(String.format("%s: %16s", NAME, id));

		if (waypoints.removeWaypoint(id) == false) {
			CommandStatus.error.addStatus(command, NAME + ": Unable to remove waypoint " + id + ", no such waypoint");
			return false;
		}

		CommandStatus.accepted.addStatus(command);
		CommandStatus.complete.addStatus(command);
		return true;
	}
}
