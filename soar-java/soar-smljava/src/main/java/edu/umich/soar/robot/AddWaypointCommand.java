/**
 * 
 */
package edu.umich.soar.robot;

import java.util.Arrays;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 *
 * Add a waypoint to the waypoint system.
 */
final public class AddWaypointCommand extends NoDDCAdapter implements Command {
	private static final Logger logger = Logger.getLogger(AddWaypointCommand.class);
	static final String NAME = "add-waypoint";
	
	static Command newInstance(OffsetPose opose, WaypointInterface waypoints) {
		return new AddWaypointCommand(opose, waypoints);
	}
	
	public AddWaypointCommand(OffsetPose opose, WaypointInterface waypoints) {
		this.opose = opose;
		this.waypoints = waypoints;
	}

	private final OffsetPose opose;
	private final WaypointInterface waypoints;

	@Override
	public boolean execute(Agent agent, Identifier command) {
		String id = command.GetParameterValue("id");
		if (id == null) {
			CommandStatus.error.addStatus(command, NAME + ": No id on command");
			return false;
		}

		if (opose == null) {
			throw new AssertionError();
		}
		
		double[] pos = Arrays.copyOf(opose.getPose().pos, opose.getPose().pos.length);
		try {
			pos[0] = Double.parseDouble(command.GetParameterValue("x"));
		} catch (NullPointerException ignored) {
			// no x param is ok, use current
		} catch (NumberFormatException e) {
			CommandStatus.error.addStatus(command, NAME + ": Unable to parse x: " + command.GetParameterValue("x"));
			return false;
		}

		try {
			pos[1] = Double.parseDouble(command.GetParameterValue("y"));
		} catch (NullPointerException ignored) {
			// no y param is ok, use current
		} catch (NumberFormatException e) {
			CommandStatus.error.addStatus(command, NAME + ": Unable to parse y: " + command.GetParameterValue("y"));
			return false;
		}

		logger.debug(String.format(NAME + ": %16s %10.3f %10.3f", id, pos[0], pos[1]));
		waypoints.addWaypoint(pos, id);

		CommandStatus.accepted.addStatus(command);
		CommandStatus.complete.addStatus(command);
		return true;
	}
}
