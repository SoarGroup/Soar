/**
 * 
 */
package org.msoar.sps.control.io.o;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

class EnableWaypointCommand implements Command {
	private static Logger logger = Logger.getLogger(EnableWaypointCommand.class);
	
	public SplinterInput execute(SplinterInput input, Identifier commandwme, pose_t pose, OutputLinkManager outputLinkManager) {
		String id = commandwme.GetParameterValue("id");
		if (id == null) {
			logger.warn("No id on enable-waypoint command");
			commandwme.AddStatusError();
			return input;
		}

		logger.debug(String.format("enable-waypoint: %16s", id));

		if (outputLinkManager.waypointsIL.enable(id, pose) == false) {
			logger.warn("Unable to enable waypoint " + id + ", no such waypoint");
			commandwme.AddStatusError();
			return input;
		}

		commandwme.AddStatusComplete();
		return input;
	}
}