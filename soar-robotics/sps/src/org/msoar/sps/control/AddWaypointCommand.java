/**
 * 
 */
package org.msoar.sps.control;

import java.util.Arrays;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

final class AddWaypointCommand implements Command {
	private static final Logger logger = Logger.getLogger(AddWaypointCommand.class);
	
	public CommandStatus execute(InputLinkInterface inputLink, Identifier command, pose_t pose, OutputLinkManager outputLinkManager) {
		String id = command.GetParameterValue("id");
		if (id == null) {
			logger.warn("No id on add-waypoint command");
			return CommandStatus.error;
		}

		if (pose == null) {
			logger.error("add-waypoint called with no current pose");
			return CommandStatus.error;
		}
		
		double[] pos = Arrays.copyOf(pose.pos, pose.pos.length);
		try {
			pos[0] = Double.parseDouble(command.GetParameterValue("x"));
		} catch (NullPointerException ignored) {
			// no x param is ok, use current
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse x: " + command.GetParameterValue("x"));
			return CommandStatus.error;
		}

		try {
			pos[1] = Double.parseDouble(command.GetParameterValue("y"));
		} catch (NullPointerException ignored) {
			// no y param is ok, use current
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse y: " + command.GetParameterValue("y"));
			return CommandStatus.error;
		}

		logger.debug(String.format("add-waypoint: %16s %10.3f %10.3f", id, pos[0], pos[1]));
		inputLink.addWaypoint(pos, id, outputLinkManager.useFloatYawWmes);

		return CommandStatus.complete;
	}

	public boolean isInterruptable() {
		return false;
	}

	public boolean modifiesInput() {
		return false;
	}

	public void updateInput(SplinterInput input) {
		assert false;
	}
}