/**
 * 
 */
package org.msoar.sps.control.io.o;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

class MoveCommand implements Command {
	private static Logger logger = Logger.getLogger(MoveCommand.class);
	
	public SplinterInput execute(SplinterInput input, Identifier commandwme, pose_t pose, OutputLinkManager outputLinkManager) {
		if (input != null) {
			logger.warn("Move command received but motors already have orders");
			commandwme.AddStatusError();
			return input;
		}

		String direction = commandwme.GetParameterValue("direction");
		if (direction == null) {
			logger.warn("No direction on move command");
			commandwme.AddStatusError();
			return input;
		}

		double throttle = 0;
		try {
			throttle = Double.parseDouble(commandwme.GetParameterValue("throttle"));
		} catch (NullPointerException ex) {
			logger.warn("No throttle on move command");
			commandwme.AddStatusError();
			return input;
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse throttle: " + commandwme.GetParameterValue("throttle"));
			commandwme.AddStatusError();
			return input;
		}

		throttle = Math.max(throttle, 0);
		throttle = Math.min(throttle, 1.0);

		logger.debug(String.format("move: %10s %10.3f", direction, throttle));

		if (direction.equals("backward")) {
			input = new SplinterInput(throttle * -1);
		} else if (direction.equals("forward")) {
			input = new SplinterInput(throttle);
		} else if (direction.equals("stop")) {
			input = new SplinterInput(0);
		} else {
			logger.warn("Unknown direction on move command: " + commandwme.GetParameterValue("direction"));
			commandwme.AddStatusError();
			return input;
		}

		commandwme.AddStatusComplete();
		return input;
	}
}