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
 * Disables a waypoint.
 */
final public class DisableWaypointCommand extends NoDDCAdapter implements Command {
	private static final Logger logger = Logger.getLogger(DisableWaypointCommand.class);
	public static final String NAME = "disable-waypoint";
	
	public static Command newInstance(WaypointInterface waypoints) {
		return new DisableWaypointCommand(waypoints);
	}
	
	public DisableWaypointCommand(WaypointInterface waypoints) {
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

		if (waypoints.disableWaypoint(id) == false) {
			logger.warn(NAME + ": Unable to disable waypoint " + id + ", no such waypoint");
			CommandStatus.error.addStatus(agent, command);
			return false;
		}

		CommandStatus.accepted.addStatus(agent, command);
		CommandStatus.complete.addStatus(agent, command);
		return true;
	}

}
