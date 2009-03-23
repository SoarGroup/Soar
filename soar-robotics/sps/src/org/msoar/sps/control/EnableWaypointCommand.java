/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Identifier;

final class EnableWaypointCommand implements Command {
	private static final Logger logger = Logger.getLogger(EnableWaypointCommand.class);
	static final String NAME = "enable-waypoint";

	public CommandStatus execute(InputLinkInterface inputLink, Identifier command, SplinterState splinter, OutputLinkManager outputLinkManager) {
		String id = command.GetParameterValue("id");
		if (id == null) {
			logger.warn(NAME + ": No id on command");
			return CommandStatus.error;
		}

		logger.debug(String.format(NAME + ": %16s", id));

		if (inputLink.enableWaypoint(id, splinter) == false) {
			logger.warn(NAME + ": Unable to enable waypoint " + id + ", no such waypoint");
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