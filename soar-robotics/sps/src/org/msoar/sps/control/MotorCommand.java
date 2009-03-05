/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

class MotorCommand implements Command {
	private static final Logger logger = Logger.getLogger(MotorCommand.class);
	double[] motorThrottle = new double[2];
	
	public CommandStatus execute(Identifier command, pose_t pose, OutputLinkManager outputLinkManager) {
		try {
			motorThrottle[0] = Double.parseDouble(command.GetParameterValue("left"));
		} catch (NullPointerException ex) {
			logger.warn("No left on motor command");
			return CommandStatus.error;
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse left: " + command.GetParameterValue("left"));
			return CommandStatus.error;
		}

		try {
			motorThrottle[1] = Double.parseDouble(command.GetParameterValue("right"));
		} catch (NullPointerException ex) {
			logger.warn("No right on motor command");
			return CommandStatus.error;
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse right: " + command.GetParameterValue("right"));
			return CommandStatus.error;
		}

		motorThrottle[0] = Math.max(motorThrottle[0], -1.0);
		motorThrottle[0] = Math.min(motorThrottle[0], 1.0);

		motorThrottle[1] = Math.max(motorThrottle[1], -1.0);
		motorThrottle[1] = Math.min(motorThrottle[1], 1.0);

		logger.debug(String.format("motor: %10.3f %10.3f", motorThrottle[0], motorThrottle[1]));
		
		return CommandStatus.executing;
	}

	public boolean isInterruptable() {
		return false;
	}

	public boolean modifiesInput() {
		return true;
	}

	public void updateInput(SplinterInput input) {
		input.motor(motorThrottle);
	}
}