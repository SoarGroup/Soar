/**
 * 
 */
package org.msoar.sps.control.o;

import java.util.Arrays;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

class AddWaypointCommand implements Command {
	private static Logger logger = Logger.getLogger(AddWaypointCommand.class);
	
	public SplinterInput execute(SplinterInput input, Identifier commandwme, pose_t pose, OutputLinkManager outputLinkManager) {
		String id = commandwme.GetParameterValue("id");
		if (id == null) {
			logger.warn("No id on add-waypoint command");
			commandwme.AddStatusError();
			return input;
		}

		if (pose == null) {
			logger.error("add-waypoint called with no current pose");
			commandwme.AddStatusError();
			return input;
		}
		
		double[] pos = Arrays.copyOf(pose.pos, pose.pos.length);
		try {
			pos[0] = Double.parseDouble(commandwme.GetParameterValue("x"));
		} catch (NullPointerException ignored) {
			// no x param is ok, use current
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse x: " + commandwme.GetParameterValue("x"));
			commandwme.AddStatusError();
			return input;
		}

		try {
			pos[1] = Double.parseDouble(commandwme.GetParameterValue("y"));
		} catch (NullPointerException ignored) {
			// no y param is ok, use current
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse y: " + commandwme.GetParameterValue("y"));
			commandwme.AddStatusError();
			return input;
		}

		logger.debug(String.format("add-waypoint: %16s %10.3f %10.3f", id, pos[0], pos[1]));

		outputLinkManager.waypointsIL.add(pos, id, outputLinkManager.useFloatYawWmes);

		commandwme.AddStatusComplete();
		return input;
	}
}