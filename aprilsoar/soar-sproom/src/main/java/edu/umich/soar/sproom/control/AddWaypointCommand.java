/**
 * 
 */
package edu.umich.soar.sproom.control;

import java.util.Arrays;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import sml.Identifier;
import sml.WMElement;

/**
 * @author voigtjr
 *
 * Add a waypoint to the waypoint system.
 */
public class AddWaypointCommand extends NoDDCAdapter implements Command {
	private static final Log logger = LogFactory.getLog(AddWaypointCommand.class);
	static final String NAME = "add-waypoint";
	
	static Command newInstance(OffsetPose opose, WaypointInterface waypoints) {
		return new AddWaypointCommand(opose, waypoints);
	}
	
	private AddWaypointCommand(OffsetPose opose, WaypointInterface waypoints) {
		this.opose = opose;
		this.waypoints = waypoints;
	}

	private final OffsetPose opose;
	private final WaypointInterface waypoints;

	@Override
	public boolean execute(Identifier command) {
		WMElement wme = command.FindByAttribute("id", 0);
		if (wme == null) {
			CommandStatus.error.addStatus(command, NAME + ": No id on command");
			return false;
		}
		String type = wme.GetValueType();
		String id = wme.GetValueAsString();
		
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
		waypoints.addWaypoint(pos, id, type);

		CommandStatus.accepted.addStatus(command);
		CommandStatus.complete.addStatus(command);
		return true;
	}
}
