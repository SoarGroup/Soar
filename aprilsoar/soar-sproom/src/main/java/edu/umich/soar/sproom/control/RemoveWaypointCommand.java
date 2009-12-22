/**
 * 
 */
package edu.umich.soar.sproom.control;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import sml.Identifier;

/**
 * @author voigtjr
 *
 * Remove waypoint from waypoint system.
 */
public class RemoveWaypointCommand extends NoDDCAdapter implements Command {
	private static final Log logger = LogFactory.getLog(RemoveWaypointCommand.class);
	static final String NAME = "remove-waypoint";

	static Command newInstance(WaypointInterface waypoints) {
		return new RemoveWaypointCommand(waypoints);
	}
	
	private RemoveWaypointCommand(WaypointInterface waypoints) {
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
