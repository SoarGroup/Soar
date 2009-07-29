/**
 * 
 */
package edu.umich.soar.robot;

import org.apache.log4j.Logger;

import sml.Identifier;

/**
 * @author voigtjr
 *
 * Disables a waypoint.
 */
final public class DisableWaypointCommand extends NoDDCAdapter implements Command {
	private static final Logger logger = Logger.getLogger(DisableWaypointCommand.class);
	static final String NAME = "disable-waypoint";
	
	static Command newInstance(WaypointInterface waypoints) {
		return new DisableWaypointCommand(waypoints);
	}
	
	public DisableWaypointCommand(WaypointInterface waypoints) {
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
