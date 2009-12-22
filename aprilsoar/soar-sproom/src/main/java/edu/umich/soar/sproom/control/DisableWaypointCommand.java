/**
 * 
 */
package edu.umich.soar.sproom.control;

import sml.Identifier;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * @author voigtjr
 *
 * Disables a waypoint.
 */
public class DisableWaypointCommand extends NoDDCAdapter implements Command {
	private static final Log logger = LogFactory.getLog(DisableWaypointCommand.class);
	static final String NAME = "disable-waypoint";
	
	static Command newInstance(WaypointInterface waypoints) {
		return new DisableWaypointCommand(waypoints);
	}
	
	private DisableWaypointCommand(WaypointInterface waypoints) {
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

		logger.debug(String.format(NAME + ": %16s", id));

		if (waypoints.disableWaypoint(id) == false) {
			CommandStatus.error.addStatus(command, NAME + ": Unable to disable waypoint " + id + ", no such waypoint");
			return false;
		}

		CommandStatus.accepted.addStatus(command);
		CommandStatus.complete.addStatus(command);
		return true;
	}

}
