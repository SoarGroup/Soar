/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

class RotateToCommand implements Command {
	private static final Logger logger = Logger.getLogger(RotateToCommand.class);
	
	double yaw;
	double tolerance;
	double throttle;
	
	public CommandStatus execute(Identifier command, pose_t pose, OutputLinkManager outputLinkManager) {
		try {
			yaw = Double.parseDouble(command.GetParameterValue("yaw"));
		} catch (NullPointerException ex) {
			logger.warn("No yaw on rotate-to command");
			return CommandStatus.error;
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse yaw: " + command.GetParameterValue("yaw"));
			return CommandStatus.error;
		}
		yaw = Math.toRadians(yaw);

		try {
			tolerance = Double.parseDouble(command.GetParameterValue("tolerance"));
		} catch (NullPointerException ex) {
			logger.warn("No tolerance on rotate-to command");
			return CommandStatus.error;
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse tolerance: " + command.GetParameterValue("tolerance"));
			return CommandStatus.error;
		}

		tolerance = Math.toRadians(tolerance);
		tolerance = Math.max(tolerance, 0);
		tolerance = Math.min(tolerance, Math.PI);

		try {
			throttle = Double.parseDouble(command.GetParameterValue("throttle"));
		} catch (NullPointerException ex) {
			logger.warn("No throttle on rotate-to command");
			return CommandStatus.error;
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse throttle: " + command.GetParameterValue("throttle"));
			return CommandStatus.error;
		}

		throttle = Math.max(throttle, 0);
		throttle = Math.min(throttle, 1.0);

		logger.debug(String.format("rotate-to: %10.3f %10.3f %10.3f", yaw, tolerance, throttle));
		
		return CommandStatus.accepted;
	}

	
	public boolean isInterruptable() {
		return true;
	}

	public boolean modifiesInput() {
		return true;
	}

	public void updateInput(SplinterInput input) {
		input.rotateTo(yaw, tolerance, throttle);
	}
}