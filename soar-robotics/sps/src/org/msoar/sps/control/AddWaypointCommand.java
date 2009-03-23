/**
 * 
 */
package org.msoar.sps.control;

import java.util.Arrays;

import org.apache.log4j.Logger;

import sml.Identifier;

final class AddWaypointCommand implements Command {
	private static final Logger logger = Logger.getLogger(AddWaypointCommand.class);
	static final String NAME = "add-waypoint";

	public CommandStatus execute(InputLinkInterface inputLink, Identifier command, SplinterState splinter, OutputLinkManager outputLinkManager) {
		String id = command.GetParameterValue("id");
		if (id == null) {
			logger.warn(NAME + ": No id on command");
			return CommandStatus.error;
		}

		if (splinter == null) {
			throw new AssertionError();
		}
		
		double[] pos = Arrays.copyOf(splinter.getSplinterPose().pos, splinter.getSplinterPose().pos.length);
		try {
			pos[0] = Double.parseDouble(command.GetParameterValue("x"));
		} catch (NullPointerException ignored) {
			// no x param is ok, use current
		} catch (NumberFormatException e) {
			logger.warn(NAME + ": Unable to parse x: " + command.GetParameterValue("x"));
			return CommandStatus.error;
		}

		try {
			pos[1] = Double.parseDouble(command.GetParameterValue("y"));
		} catch (NullPointerException ignored) {
			// no y param is ok, use current
		} catch (NumberFormatException e) {
			logger.warn(NAME + ": Unable to parse y: " + command.GetParameterValue("y"));
			return CommandStatus.error;
		}

		logger.debug(String.format(NAME + ": %16s %10.3f %10.3f", id, pos[0], pos[1]));
		inputLink.addWaypoint(pos, id, outputLinkManager.useFloatYawWmes);

		return CommandStatus.complete;
	}

	public boolean isInterruptable() {
		return false;
	}

	public boolean createsDDC() {
		return false;
	}

	public DifferentialDriveCommand getDDC() {
		throw new AssertionError();
	}
}