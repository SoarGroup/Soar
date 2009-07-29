/**
 * 
 */
package edu.umich.soar.robot;

import org.apache.log4j.Logger;

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
	
	private EnableWaypointCommand(WaypointInterface waypoints) {
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

		if (waypoints.enableWaypoint(id) == false) {
			CommandStatus.error.addStatus(command, NAME + ": Unable to enable waypoint " + id + ", no such waypoint");
			return false;
		}

		CommandStatus.accepted.addStatus(command);
		CommandStatus.complete.addStatus(command);
		return true;
	}
}
