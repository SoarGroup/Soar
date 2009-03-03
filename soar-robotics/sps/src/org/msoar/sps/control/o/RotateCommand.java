/**
 * 
 */
package org.msoar.sps.control.o;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

class RotateCommand implements Command {
	private static Logger logger = Logger.getLogger(RotateCommand.class);
	
	public SplinterInput execute(SplinterInput input, Identifier commandwme, pose_t pose, OutputLinkManager outputLinkManager) {
		if (input != null) {
			logger.warn("Rotate command received but motors already have orders");
			commandwme.AddStatusError();
			return input;
		}

		String direction = commandwme.GetParameterValue("direction");
		if (direction == null) {
			logger.warn("No direction on rotate command");
			commandwme.AddStatusError();
			return input;
		}

		double throttle = 0;
		try {
			throttle = Double.parseDouble(commandwme.GetParameterValue("throttle"));
		} catch (NullPointerException ex) {
			logger.warn("No throttle on rotate command");
			commandwme.AddStatusError();
			return input;
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse throttle: " + commandwme.GetParameterValue("throttle"));
			commandwme.AddStatusError();
			return input;
		}

		throttle = Math.max(throttle, 0);
		throttle = Math.min(throttle, 1.0);

		logger.debug(String.format("rotate: %10s %10.3f", direction, throttle));

		if (direction.equals("left")) {
			input = new SplinterInput(SplinterInput.Direction.left, throttle);
		} else if (direction.equals("right")) {
			input = new SplinterInput(SplinterInput.Direction.right, throttle);
		} else if (direction.equals("stop")) {
			input = new SplinterInput(0);
		} else {
			logger.warn("Unknown direction on rotate command: " + commandwme.GetParameterValue("direction"));
			commandwme.AddStatusError();
			return input;
		}

		commandwme.AddStatusComplete();
		return input;
	}
}