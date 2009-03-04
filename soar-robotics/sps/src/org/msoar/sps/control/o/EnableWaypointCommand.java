/**
 * 
 */
package org.msoar.sps.control.o;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

class EnableWaypointCommand implements Command {
	private static final Logger logger = Logger.getLogger(EnableWaypointCommand.class);
	
	public CommandStatus execute(Identifier command, pose_t pose, OutputLinkManager outputLinkManager) {
		String id = command.GetParameterValue("id");
		if (id == null) {
			logger.warn("No id on enable-waypoint command");
			return CommandStatus.error;
		}

		logger.debug(String.format("enable-waypoint: %16s", id));

		if (outputLinkManager.waypointsIL.enable(id, pose) == false) {
			logger.warn("Unable to enable waypoint " + id + ", no such waypoint");
			return CommandStatus.error;
		}

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