/**
 * 
 */
package org.msoar.sps.control.io.o;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

class RotateToCommand implements Command {
	private static Logger logger = Logger.getLogger(RotateToCommand.class);
	
	public SplinterInput execute(SplinterInput input, Identifier commandwme, pose_t pose, OutputLinkManager outputLinkManager) {
		if (input != null) {
			logger.warn("Rotate-to command received but motors already have orders");
			commandwme.AddStatusError();
			return input;
		}

		double yaw = 0;
		try {
			yaw = Double.parseDouble(commandwme.GetParameterValue("yaw"));
		} catch (NullPointerException ex) {
			logger.warn("No yaw on rotate-to command");
			commandwme.AddStatusError();
			return input;
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse yaw: " + commandwme.GetParameterValue("yaw"));
			commandwme.AddStatusError();
			return input;
		}
		yaw = Math.toRadians(yaw);

		double tolerance = 0;
		try {
			tolerance = Double.parseDouble(commandwme.GetParameterValue("tolerance"));
		} catch (NullPointerException ex) {
			logger.warn("No tolerance on rotate-to command");
			commandwme.AddStatusError();
			return input;
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse tolerance: " + commandwme.GetParameterValue("tolerance"));
			commandwme.AddStatusError();
			return input;
		}

		tolerance = Math.toRadians(tolerance);
		tolerance = Math.max(tolerance, 0);
		tolerance = Math.min(tolerance, Math.PI);

		double throttle = 0;
		try {
			throttle = Double.parseDouble(commandwme.GetParameterValue("throttle"));
		} catch (NullPointerException ex) {
			logger.warn("No throttle on rotate-to command");
			commandwme.AddStatusError();
			return input;
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse throttle: " + commandwme.GetParameterValue("throttle"));
			commandwme.AddStatusError();
			return input;
		}

		throttle = Math.max(throttle, 0);
		throttle = Math.min(throttle, 1.0);

		logger.debug(String.format("rotate-to: %10.3f %10.3f %10.3f", yaw, tolerance, throttle));

		input = new SplinterInput(yaw, tolerance, throttle);

		commandwme.AddStatusComplete();
		return input;
	}
}