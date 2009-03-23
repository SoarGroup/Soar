/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Identifier;

final class RemoveWaypointCommand implements Command {
	private static final Logger logger = Logger.getLogger(RemoveWaypointCommand.class);
	static final String NAME = "remove-waypoint";

	public CommandStatus execute(InputLinkInterface inputLink, Identifier command, SplinterState splinter, OutputLinkManager outputLinkManager) {
		String id = command.GetParameterValue("id");
		if (id == null) {
			logger.warn(NAME + ": No id on command");
			return CommandStatus.error;
		}

		logger.debug(String.format("%s: %16s", NAME, id));

		if (inputLink.removeWaypoint(id) == false) {
			logger.warn(NAME + ": Unable to remove waypoint " + id + ", no such waypoint");
			return CommandStatus.error;
		}

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