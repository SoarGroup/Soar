/**
 * 
 */
package org.msoar.sps.control.o;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

class MotorCommand implements Command {
	private static Logger logger = Logger.getLogger(MotorCommand.class);
	
	public SplinterInput execute(SplinterInput input, Identifier commandwme, pose_t pose, OutputLinkManager outputLinkManager) {
		if (input != null) {
			// This is a warning
			logger.warn("Motor command received possibly overriding previous orders");
		}

		double[] motorThrottle = { 0, 0 };

		try {
			motorThrottle[0] = Double.parseDouble(commandwme.GetParameterValue("left"));
		} catch (NullPointerException ex) {
			logger.warn("No left on motor command");
			commandwme.AddStatusError();
			return input;
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse left: " + commandwme.GetParameterValue("left"));
			commandwme.AddStatusError();
			return input;
		}

		try {
			motorThrottle[1] = Double.parseDouble(commandwme.GetParameterValue("right"));
		} catch (NullPointerException ex) {
			logger.warn("No right on motor command");
			commandwme.AddStatusError();
			return input;
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse right: " + commandwme.GetParameterValue("right"));
			commandwme.AddStatusError();
			return input;
		}

		motorThrottle[0] = Math.max(motorThrottle[0], -1.0);
		motorThrottle[0] = Math.min(motorThrottle[0], 1.0);

		motorThrottle[1] = Math.max(motorThrottle[1], -1.0);
		motorThrottle[1] = Math.min(motorThrottle[1], 1.0);

		logger.debug(String.format("motor: %10.3f %10.3f", motorThrottle[0], motorThrottle[1]));

		input = new SplinterInput(motorThrottle);

		commandwme.AddStatusComplete();
		return input;
	}
}